/*
 Copyright (C) 2011 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "polycode/core/PolySoundManager.h"
#include "polycode/core/PolyCore.h"
#include "polycode/core/PolyLogger.h"

using namespace Polycode;


SoundManager::SoundManager() {
    audioInterface = NULL;
    testVal = 0.0;
    globalVolume = 1.0;
    leftOver = 0.0;
}

void SoundManager::setGlobalVolume(Number globalVolume) {
    this->globalVolume = globalVolume;
}

void SoundManager::setListenerPosition(Vector3 position) {
    // NOAL_TODO
//	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void SoundManager::setListenerOrientation(Vector3 orientation, Vector3 upVector) {
    /*
	ALfloat ori[6];
	ori[0] = orientation.x;
	ori[1] = orientation.y;
	ori[2] = orientation.z;
	
	ori[3] = upVector.x;
	ori[4] = upVector.y;
	ori[5] = upVector.z;	
	alListenerfv(AL_ORIENTATION,ori);
     */
    // NOAL_TODO
}

bool SoundManager::recordSound(unsigned int rate, unsigned int sampleSize) {
    // NOAL_TODO
    /*
    if(captureDevice) {
        Logger::log("Error: Audio capture already in progress\n");
        return false;
    }
    
    captureDevice = alcCaptureOpenDevice(NULL, rate, AL_FORMAT_STEREO16, sampleSize);
    if (alGetError() != AL_NO_ERROR) {
        captureDevice = NULL;
        return false;
    }
    recordingBufferRate = rate;
    
    recordingBuffer = (ALbyte*) malloc(1);
    recordingBufferSize = 0;
    
    alcCaptureStart(captureDevice);
    return true;
     */
    return false;
}

Sound *SoundManager::stopRecording(bool generateFloatBuffer) {
    /*
    if(!captureDevice) {
        Logger::log("No recording in process\n");
        return NULL;
    }
    alcCaptureStop(captureDevice);
    alcCaptureCloseDevice(captureDevice);
    captureDevice = NULL;
    
    Sound *newSound = new Sound(recordingBufferSize, (const char*)recordingBuffer, 2, recordingBufferRate, 16, generateFloatBuffer);
    
    free(recordingBuffer);
    
    return newSound;
     */
        // NOAL_TODO
    return NULL;
}

void SoundManager::registerSound(Sound *sound) {
    sounds.push_back(sound);
}

void SoundManager::unregisterSound(Sound *sound) {
    for(int i=0; i < sounds.size(); i++) {
        if(sounds[i] == sound) {
            sounds.erase(sounds.begin()+i);
            return;
        }
    }
}

void SoundManager::setAudioInterface(AudioInterface *audioInterface) {
    this->audioInterface = audioInterface;
}


AudioInterface::AudioInterface() {
    readOffset = 0;
    writeOffset = 0;
    memset(bufferData, 0, sizeof(int16_t) * POLY_FRAMES_PER_BUFFER*POLY_CIRCULAR_BUFFER_SIZE);
}

void AudioInterface::addToBuffer(int16_t *data, unsigned int count) {
    for(int i=0; i < count; i++) {
        for(int b=0; b < POLY_NUM_CHANNELS; b++) {
            bufferData[b][writeOffset] = data[(i*POLY_NUM_CHANNELS)+b];
        }
        writeOffset++;
        if(writeOffset >= POLY_FRAMES_PER_BUFFER * POLY_CIRCULAR_BUFFER_SIZE) {
            writeOffset = 0;
        }
        
    }
}

inline Number mixSamples(Number A, Number B) {

    if (A < 0 && B < 0 ) {
        return  (A + B) - (A * B)/-1.0;
    } else if (A > 0 && B > 0 ) {
        return (A + B) - (A * B)/1.0;
    } else {
        return A + B;
    }
}

void SoundManager::Update() {
    Number elapsed = Services()->getCore()->getElapsed();
    
    if(audioInterface) {

        // mix sounds
        unsigned int numSamples = ((Number)POLY_AUDIO_FREQ)*(elapsed);
        
        // align to 64 samples
       // numSamples = numSamples-(numSamples%64);
        
        
        if(numSamples > POLY_MIX_BUFFER_SIZE) {
            numSamples = POLY_MIX_BUFFER_SIZE;
        }

        for(int i=0; i < sounds.size(); i++) {
            sounds[i]->updateStream(numSamples);
        }
        
        int16_t *bufferPtr = mixBuffer;
        for(int i=0; i < numSamples; i++) {
            
            Number mixResults[POLY_NUM_CHANNELS];
            memset(mixResults, 0, sizeof(Number) * POLY_NUM_CHANNELS);
            
            int mixNum = 0;
            for(int i=0; i < sounds.size(); i++) {
                if(sounds[i]->isPlaying()) {
                    for(int c=0; c < POLY_NUM_CHANNELS; c++) {
                        Number A = mixResults[c];
                        Number B = sounds[i]->getSampleAsNumber(sounds[i]->getOffset(), c);
                        
                        if(mixNum == 0) {
                            mixResults[c] = B;
                        } else {
                            mixResults[c] = mixSamples(A, B);
                        }
                    }
                    sounds[i]->setOffset(sounds[i]->getOffset()+1);
                    mixNum++;
                }
            }
            
            for(int c=0; c < POLY_NUM_CHANNELS; c++) {
                *bufferPtr = (int16_t)(((Number)INT16_MAX) * (mixResults[c] * globalVolume));
                bufferPtr++;
            }
        }
        
        audioInterface->addToBuffer(mixBuffer, numSamples);
    }
}

SoundManager::~SoundManager() {
    delete audioInterface;
}
