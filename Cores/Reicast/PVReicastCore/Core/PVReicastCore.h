//
//  PVReicastCore.h
//  PVReicast
//
//  Created by Joseph Mattiello on 4/6/18.
//  Copyright © 2018 Provenance. All rights reserved.
//

#import <Foundation/Foundation.h>
@import PVCoreBridge;
#import <PVSupport/PVEmulatorCore.h>



#define GET_CURRENT_AND_RETURN(...) __strong __typeof__(_current) current = _current; if(current == nil) return __VA_ARGS__;
#define GET_CURRENT_OR_RETURN(...)  __strong __typeof__(_current) current = _current; if(current == nil) return __VA_ARGS__;

PVCORE_DIRECT_MEMBERS
@interface PVReicastCore : PVEmulatorCore <PVDreamcastSystemResponderClient>
{
	uint8_t padData[4][PVDreamcastButtonCount];
	int8_t xAxis[4];
	int8_t yAxis[4];
	//    int videoWidth;
	//    int videoHeight;
	//    int videoBitDepth;
	int videoDepthBitDepth; // eh

	float sampleRate;

	BOOL isNTSC;
@public
    dispatch_queue_t _callbackQueue;
}

@property (nonatomic, assign) int videoWidth;
@property (nonatomic, assign) int videoHeight;
@property (nonatomic, assign) int videoBitDepth;

@end

extern __weak PVReicastCore *_current;
