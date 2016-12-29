#include "imagebuffer.h"

#include <QDebug>

ImageBuffer::ImageBuffer(int bufferSize, int type, bool dropFrame) 
	: _bufferSize(bufferSize)
	, _bufferType(type)
	, _dropFrame(dropFrame)
{
    if(_bufferType == bufferType::displayBuffer)
    {
        _bufferSize = 1;
    }
    // Semaphore initializations
    freeSlots = new QSemaphore(_bufferSize);
    usedSlots = new QSemaphore(0);
    clearBuffer1 = new QSemaphore(1);
    clearBuffer2 = new QSemaphore(1);

	imageQueue = boost::circular_buffer<ImagePacket>(_bufferSize);
} // ImageBuffer constructor

void ImageBuffer::addFrame(const ImagePacket& frame)
{
    if(_bufferType == bufferType::recordBuffer)
    {
        // Acquire semaphore
        clearBuffer1->acquire();
        // If frame dropping is enabled, do not block if buffer is full
        if(_dropFrame)
        {
            // Try and acquire semaphore to add frame
            if(freeSlots->tryAcquire())
            {
                // Add frame to queue
                imageQueueProtect.lock();
                imageQueue.push_back(frame);
                imageQueueProtect.unlock();
                // Release semaphore
                usedSlots->release();
            }
        }
        // If buffer is full, wait on semaphore
        else
        {
            // Acquire semaphore
            freeSlots->acquire();
            // Add frame to queue
            imageQueueProtect.lock();
            imageQueue.push_back(frame);
			int buffersize = imageQueue.size();
            imageQueueProtect.unlock();
            // Release semaphore
            usedSlots->release();
        }
        // Release semaphore
        clearBuffer1->release();
    }
    else
    //display buffer不做缓存功能，仅作为两个线程的公共访问区
    {
        //获取信号量
        clearBuffer1->acquire();
        //display 需要获取最新图像，故不保存未显示的图像
        imageQueueProtect.lock();
		//qDebug() << imageQueue.size();
		imageQueue.push_back(frame);
        imageQueueProtect.unlock();
        //释放信号量
        clearBuffer1->release();
    }
}
// addFrame()

ImagePacket ImageBuffer::getFrameToWrite()
{
    // Acquire semaphores
    clearBuffer2->acquire();
    usedSlots->acquire();

    // Take frame from queue
    imageQueueProtect.lock();
    // Temporary data
	int buffersize = imageQueue.size();
    ImagePacket tempFrame = imageQueue.front();
	imageQueue.pop_front();
    imageQueueProtect.unlock();
    // Release semaphores
    freeSlots->release();
    clearBuffer2->release();
    // Return frame to caller
    return tempFrame;
} // getFrameToWrite()

ImagePacket ImageBuffer::getFrameToDisplay()
{
	// Acquire semaphores
    clearBuffer2->acquire();
    // Take frame from queue
	imageQueueProtect.lock();
	// 仅读取最新的帧，但不做帧弹出
    ImagePacket tempFrame = imageQueue.front();
	imageQueueProtect.unlock();
	// Release semaphores
	clearBuffer2->release();
	// Return frame to caller
	return tempFrame;;
}

void ImageBuffer::clearBuffer()
{
    // Check if buffer is not empty
    if(imageQueue.size()!=0)
    {
        // Stop adding frames to buffer
        clearBuffer1->acquire();
        // Stop taking frames from buffer
        clearBuffer2->acquire();
        // Release all remaining slots in queue
        freeSlots->release(imageQueue.size());
        // Acquire all queue slots
        freeSlots->acquire(_bufferSize);
        // Reset usedSlots to zero
        usedSlots->acquire(imageQueue.size());
        // Clear buffer
        imageQueue.clear();
        // Release all slots
        freeSlots->release(_bufferSize);
        // Allow getFrameToWrite() to resume
        clearBuffer2->release();
        // Allow addFrame() to resume
        clearBuffer1->release();
        qDebug() << "Image buffer successfully cleared.";
    }
    else
        qDebug() << "WARNING: Could not clear image buffer: already empty.";
} // clearBuffer()

int ImageBuffer::getSizeOfImageBuffer()
{
    return imageQueue.size();
} // getSizeOfImageBuffer()

bool ImageBuffer::isBufferEmpty()
{
	return imageQueue.empty();
}
