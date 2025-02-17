//
//  PVReicast+CoreAudio.h
//  PVReicast
//
//  Created by Joseph Mattiello on 11/1/18.
//  Copyright © 2018 Provenance. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>
@import PVCoreBridge;
#import <PVSupport/CARingBuffer.h>
#import <PVSupport/OEGameAudio.h>

typedef struct ReicastAUGraphPlayer {
    AudioStreamBasicDescription streamFormat;
    AUGraph graph;
    AudioUnit inputUnit;
    AudioUnit outputUnit;
    AudioUnit converterUnit;

    AudioBufferList *inputBuffer;
    CARingBuffer *ringBuffer;
    Float64 firstInputSampleTime;
    Float64 firstOutputSampleTime;
    Float64 inToOutSampleTimeOffset;
} ReicastAUGraphPlayer;

static ReicastAUGraphPlayer player;

OSStatus InputRenderProc(void *inRefCon,
                         AudioUnitRenderActionFlags *ioActionFlags,
                         const AudioTimeStamp *inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList * ioData);
OSStatus GraphRenderProc(void *inRefCon,
                         AudioUnitRenderActionFlags *ioActionFlags,
                         const AudioTimeStamp *inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList * ioData);

void CreateMyAUGraph(ReicastAUGraphPlayer *player);
void InitAUPlayer(ReicastAUGraphPlayer *player);
static void CheckError(OSStatus error, const char *operation);
