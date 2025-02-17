/*
 Copyright (c) 2013, OpenEmu Team
 

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

@import PVAudio;
@import PVSupport;
@import libstella;
@import PVStellaCPP;
@import PVLoggingObjC;
#if !TARGET_OS_WATCH
@import GameController;
#endif
@import PVCoreBridge;
@import PVObjCUtils;
@import PVEmulatorCore;
#import <libstella/libretro/libretro.h>
#import <libstella/libstella.h>

#import "PVStellaBridge.h"

#if __has_include(<OpenGLES/gltypes.h>)
#import <OpenGLES/gltypes.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <OpenGLES/EAGL.h>
#elif !TARGET_OS_WATCH
#import <OpenGL/OpenGL.h>
#import <OpenGL/GL3.h>
#import <GLUT/GLUT.h>
#endif

@interface PVStellaBridge () <GameWithCheat> {
    stellabuffer_t *_videoBuffer;
    int _videoWidth, _videoHeight;
    int16_t _pad[NUMBER_OF_PADS][NUMBER_OF_PAD_INPUTS];

    // RETRO_REGION_NTSC, RETRO_REGION_PAL
    unsigned region;
}
@property (nonatomic, strong) NSMutableArray<NSString*>* cheats;
@property (readwrite, nonatomic, copy) PVStellaBridgeOptionHandler optionHandler;

@end

static __weak PVStellaBridge *_current;

@implementation PVStellaBridge

#pragma mark - Static callbacks
static void audio_callback(int16_t left, int16_t right) {
    __strong PVStellaBridge *strongCurrent = _current;

	[[strongCurrent ringBufferAtIndex:0] write:&left size:2];
    [[strongCurrent ringBufferAtIndex:0] write:&right size:2];

    strongCurrent = nil;
}

static size_t audio_batch_callback(const int16_t *data, size_t frames) {
    __strong PVStellaBridge *strongCurrent = _current;

    [[strongCurrent ringBufferAtIndex:0] write:data size:frames << 2];

    strongCurrent = nil;
    
    return frames;
}

static dispatch_queue_t memcpy_queue =
dispatch_queue_create("stella memcpy queue", dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_CONCURRENT, QOS_CLASS_USER_INTERACTIVE, 0));

static void video_callback(const void *data, unsigned width, unsigned height, size_t pitch) {
    __strong PVStellaBridge *strongCurrent = _current;

    strongCurrent->_videoWidth  = width;
    strongCurrent->_videoHeight = height;

    dispatch_apply(height, memcpy_queue, ^(size_t y) {
        const stellabuffer_t *src = (stellabuffer_t*)data + y * (pitch >> STELLA_PITCH_SHIFT); //pitch is in bytes not pixels
        
        //uint16_t *dst = current->videoBuffer + y * current->videoWidth;
        stellabuffer_t *dst = strongCurrent->_videoBuffer + y * width;
        
        memcpy(dst, src, sizeof(stellabuffer_t)*width);
    });
    
    strongCurrent = nil;
}

static void input_poll_callback(void) {
	DLOG(@"poll callback");
}

static int16_t input_state_callback(unsigned port, unsigned device, unsigned index, unsigned _id) {
//    DLOG(@"polled input: port: %d device: %d id: %d", port, device, _id);
    
    __strong PVStellaBridge *strongCurrent = _current;
    int16_t value = 0;
    
    if (port == 0 & device == RETRO_DEVICE_JOYPAD)
    {
        value = strongCurrent->_pad[0][_id];
    }
    else if(port == 1 & device == RETRO_DEVICE_JOYPAD)
    {
        if (value == 0)
        {
            value = strongCurrent->_pad[1][_id];
        }
    }
    
    strongCurrent = nil;
    
    return value;
}

static bool environment_callback(unsigned cmd, void *data) {
    __strong PVStellaBridge *strongCurrent = _current;
    
    switch(cmd) {
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY : {
            NSString *appSupportPath = [strongCurrent BIOSPath];
            
            *(const char **)data = [appSupportPath UTF8String];
            DLOG(@"Environ SYSTEM_DIRECTORY: \"%@\".\n", appSupportPath);
            return true;
        }
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            enum retro_pixel_format pix_fmt = *(const enum retro_pixel_format*)data;
            switch (pix_fmt)
            {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    NSLog(@"Environ SET_PIXEL_FORMAT: 0RGB1555");
                    break;

                case RETRO_PIXEL_FORMAT_RGB565:
                    NSLog(@"Environ SET_PIXEL_FORMAT: RGB565");
                    break;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    NSLog(@"Environ SET_PIXEL_FORMAT: XRGB8888");
                    break;

                default:
                    return false;
            }
            //currentPixFmt = pix_fmt;
            break;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE: {
            struct retro_variable *var = (struct retro_variable*)data;
            NSString *varS = [NSString stringWithUTF8String:var->key];
            id _Nullable oValue = strongCurrent.optionHandler(varS); //[strongCurrent getVariable:varS];
            
            if ([oValue isKindOfClass:[NSString class]]) {
                NSString *value = oValue;
                if(oValue && value && value.length) {
                    var->value = [value cStringUsingEncoding:kCFStringEncodingUTF8];
                    return true;
                } else {
                    return false;
                }
            } else if ([oValue isKindOfClass:[NSNumber class]]) {
                NSNumber *value = oValue;
                if(value) {
                    var->value = [[value stringValue] cStringUsingEncoding:kCFStringEncodingUTF8];
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }

        }
        default : {
            DLOG(@"Environ UNSUPPORTED (#%u).\n", cmd);
            return false;
        }
    }
    
    strongCurrent = nil;
    
    return true;
}


static void loadSaveFile(const char* path, int type) {
    FILE *file;
    
    file = fopen(path, "rb");
    if ( !file ) {
        return;
    }
    
    size_t size = retro_get_memory_size(type);
    void *data  = retro_get_memory_data(type);
    
    if (size == 0 || !data) {
        fclose(file);
        return;
    }
    
    size_t rc = fread(data, sizeof(uint8_t), size, file);
    if ( rc != size ) {
        DLOG(@"Couldn't load save file.");
    }
    
    DLOG(@"Loaded save file: %s", path);
    fclose(file);
}

static void writeSaveFile(const char* path, int type) {
    size_t size = retro_get_memory_size(type);
    void *data = retro_get_memory_data(type);
    
    if ( data && size > 0 ) {
        FILE *file = fopen(path, "wb");
        if ( file != NULL ) {
            DLOG(@"Saving state %s. Size: %d bytes.", path, (int)size);
            retro_serialize(data, size);
            if ( fwrite(data, sizeof(uint8_t), size, file) != size ) {
                DLOG(@"Did not save state properly.");
            }
            fclose(file);
        }
    }
}

- (instancetype)initWithOptionHandler:(PVStellaBridgeOptionHandler)optionHandler {
    if((self = [super init])) {
        _current = self;
        self.optionHandler = optionHandler;
    }

	return self;
}

#pragma mark - Exectuion

- (void)resetEmulation {
//    [super resetEmulation];
    retro_reset();
}

- (void)stopEmulation {
    if ([self.batterySavesPath length]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:self.batterySavesPath withIntermediateDirectories:YES attributes:nil error:NULL];
        NSString *filePath = [self.batterySavesPath stringByAppendingPathComponent:[self.romName stringByAppendingPathExtension:@"sav"]];
        [self writeSaveFile:filePath forType:RETRO_MEMORY_SAVE_RAM];
    }
    
    [super stopEmulation];
    
    double delayInSeconds = 0.1;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        retro_unload_game();
        retro_deinit();
    });
    self->region = RETRO_REGION_NTSC;
}

- (void)dealloc {
    dispatch_sync(dispatch_get_main_queue(), ^{
        if(self->_videoBuffer) {
            free(self->_videoBuffer);
        }
    });
}

- (void)executeFrame {
#if !TARGET_OS_WATCH
    if (self.controller1 || self.controller2) {
        [self pollControllers];
    }
#endif
    retro_run();
}

- (void)executeFrameSkippingFrame: (BOOL) skip {
#if !TARGET_OS_WATCH
    if (!skip && (self.controller1 || self.controller2)) {
        [self pollControllers];
    }
#endif
    retro_run();
}

- (BOOL)loadFileAtPath:(NSString *)path error:(NSError **)error {
	memset(_pad, 0, sizeof(int16_t) * NUMBER_OF_PADS * NUMBER_OF_PAD_INPUTS);
    if(self->_videoBuffer) {
        free(self->_videoBuffer);
    }
    self->_videoBuffer = (stellabuffer_t*)malloc(STELLA_WIDTH * STELLA_HEIGHT * 4);

    const void *data;
    size_t size;
    self.romName = [[[path lastPathComponent] componentsSeparatedByString:@"."] objectAtIndex:0]; //[path copy];
    
    //load cart, read bytes, get length
    NSData* dataObj = [NSData dataWithContentsOfFile:[path stringByStandardizingPath]];
    if(dataObj == nil) return false;
    size = [dataObj length];
    data = (uint8_t*)[dataObj bytes];
    const char *meta = NULL;
    
    //memory.copy(data, size);
    retro_set_environment(environment_callback);
	retro_init();
	
    retro_set_audio_sample(audio_callback);
    retro_set_audio_sample_batch(audio_batch_callback);
    retro_set_video_refresh(video_callback);
    retro_set_input_poll(input_poll_callback);
    retro_set_input_state(input_state_callback);
    
    
    const char *fullPath = [path UTF8String];
    
    struct retro_game_info info = {NULL};
    info.path = fullPath;
    info.data = data;
    info.size = size;
    info.meta = meta;
    
    BOOL loaded = retro_load_game(&info);

    if (loaded) {
        if ([self.batterySavesPath length]) {
            [[NSFileManager defaultManager] createDirectoryAtPath:self.batterySavesPath 
                                      withIntermediateDirectories:YES
                                                       attributes:nil
                                                            error:NULL];

            NSString *filePath = [self.batterySavesPath stringByAppendingPathComponent:[self.romName stringByAppendingPathExtension:@"sav"]];
            
            [self loadSaveFile:filePath forType:RETRO_MEMORY_SAVE_RAM];
        }
        
        struct retro_system_av_info info;
        retro_get_system_av_info(&info);
        
        self->_frameInterval = info.timing.fps;
        self->_sampleRate = info.timing.sample_rate;

        uint currentRegion = retro_get_region();
        if (currentRegion == RETRO_REGION_PAL) {
            self->region = RETRO_REGION_PAL;
        } else {
            self->region = RETRO_REGION_NTSC;
        }

        retro_run();
        
        return YES;
    } else {
        if(error) {
            *error = [NSError errorWithDomain:@"" code:-1 userInfo:@{
                NSLocalizedDescriptionKey : @"Failed to load ROM",
                NSLocalizedRecoverySuggestionErrorKey : @"Stella could not load the ROM. The core does not supply additional information."
            }];
        }

        ELOG(@"Stella failed to load ROM.")

        return NO;

    }
}

#pragma mark - Input
#if !TARGET_OS_WATCH

- (void)pollControllers {
    for (NSInteger playerIndex = 0; playerIndex < 2; playerIndex++) {
        GCController *controller = nil;
        
        if (self.controller1 && playerIndex == 0) {
            controller = self.controller1;
        }
        else if (self.controller2 && playerIndex == 1)
        {
            controller = self.controller2;
        }
        
        if ([controller extendedGamepad]) {
            GCExtendedGamepad *gamepad     = [controller extendedGamepad];
            GCControllerDirectionPad *dpad = [gamepad dpad];
            
            /* TODO: To support paddles we would need to circumvent libRetro's emulation of analog controls or drop libRetro and talk to stella directly like OpenEMU did */
            
            // D-Pad
            float deadZone = 0.1;
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_UP]    = (dpad.up.isPressed    || gamepad.leftThumbstick.up.value > deadZone);
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_DOWN]  = (dpad.down.isPressed  || gamepad.leftThumbstick.down.value > deadZone);
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_LEFT]  = (dpad.left.isPressed  || gamepad.leftThumbstick.left.value > deadZone);
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_RIGHT] = (dpad.right.isPressed || gamepad.leftThumbstick.right.value > deadZone);

			// #688, use second thumb to control second player input if no controller active
			// some games used both joysticks for 1 player optionally
			if(playerIndex == 0 && self.controller2 == nil) {
				_pad[1][RETRO_DEVICE_ID_JOYPAD_UP]    = gamepad.rightThumbstick.up.isPressed;
				_pad[1][RETRO_DEVICE_ID_JOYPAD_DOWN]  = gamepad.rightThumbstick.down.isPressed;
				_pad[1][RETRO_DEVICE_ID_JOYPAD_LEFT]  = gamepad.rightThumbstick.left.isPressed;
				_pad[1][RETRO_DEVICE_ID_JOYPAD_RIGHT] = gamepad.rightThumbstick.right.isPressed;
			}

            // Fire
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_B] = gamepad.buttonA.isPressed;
            // Trigger
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_A] =  gamepad.buttonB.isPressed || gamepad.rightTrigger.isPressed;
            // Booster
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_X] = gamepad.buttonX.isPressed || gamepad.buttonY.isPressed || gamepad.leftTrigger.isPressed;
            
            // Reset
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_START]  = gamepad.rightShoulder.isPressed;
            
            // Select
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_SELECT] = gamepad.leftShoulder.isPressed;
   
            /*
             #define RETRO_DEVICE_ID_JOYPAD_B        0 == JoystickZeroFire1
             #define RETRO_DEVICE_ID_JOYPAD_Y        1 == Unmapped
             #define RETRO_DEVICE_ID_JOYPAD_SELECT   2 == ConsoleSelect
             #define RETRO_DEVICE_ID_JOYPAD_START    3 == ConsoleReset
             #define RETRO_DEVICE_ID_JOYPAD_UP       4 == Up
             #define RETRO_DEVICE_ID_JOYPAD_DOWN     5 == Down
             #define RETRO_DEVICE_ID_JOYPAD_LEFT     6 == Left
             #define RETRO_DEVICE_ID_JOYPAD_RIGHT    7 == Right
             #define RETRO_DEVICE_ID_JOYPAD_A        8 == JoystickZeroFire2
             #define RETRO_DEVICE_ID_JOYPAD_X        9 == JoystickZeroFire3
             #define RETRO_DEVICE_ID_JOYPAD_L       10 == ConsoleLeftDiffA
             #define RETRO_DEVICE_ID_JOYPAD_R       11 == ConsoleRightDiffA
             #define RETRO_DEVICE_ID_JOYPAD_L2      12 == ConsoleLeftDiffB
             #define RETRO_DEVICE_ID_JOYPAD_R2      13 == ConsoleRightDiffB
             #define RETRO_DEVICE_ID_JOYPAD_L3      14 == ConsoleColor
             #define RETRO_DEVICE_ID_JOYPAD_R3      15 == ConsoleBlackWhite
             */
        }
#if TARGET_OS_TV
        else if ([controller microGamepad]) {
            GCMicroGamepad *gamepad = [controller microGamepad];
            GCControllerDirectionPad *dpad = [gamepad dpad];
            
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_UP]    = dpad.up.value > 0.5;
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_DOWN]  = dpad.down.value > 0.5;
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_LEFT]  = dpad.left.value > 0.5;
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_RIGHT] = dpad.right.value > 0.5;

            // Fire
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_B] = gamepad.buttonX.isPressed;
            // Trigger
            _pad[playerIndex][RETRO_DEVICE_ID_JOYPAD_A] = gamepad.buttonA.isPressed;
        }
#endif
    }
}

#endif

#pragma mark - Video
- (const void *)videoBuffer
{
    return self->_videoBuffer;
}

- (CGRect)screenRect {
//    __strong PVStellaGameCore *strongCurrent = _current;

    //return OEIntRectMake(0, 0, strongCurrent->_videoWidth, strongCurrent->_videoHeight);
    return CGRectMake(0, 0, self->_videoWidth, self->_videoHeight);
}

- (CGSize)bufferSize {
    return CGSizeMake(STELLA_WIDTH, STELLA_HEIGHT);
    
//    __strong PVStellaGameCore *strongCurrent = _current;
    //return CGSizeMake(strongCurrent->_videoWidth, strongCurrent->_videoHeight);
}

- (CGSize)aspectSize {
//    return CGSizeMake(4, 3);
    return CGSizeMake(self->_videoWidth * (12.0/7.0), self->_videoHeight);
//    return CGSizeMake(STELLA_WIDTH * 2, STELLA_HEIGHT);
}

#pragma mark - Video
#if !TARGET_OS_WATCH

- (GLenum)pixelFormat
{
    return STELLA_PIXEL_FORMAT;
}

- (GLenum)pixelType
{
    return  STELLA_PIXEL_TYPE;
}

- (GLenum)internalPixelFormat
{
    return STELLA_INTERNAL_FORMAT;
}
#endif

#pragma mark - Audio
- (double)audioSampleRate {
    return (self->_sampleRate > 0) ? self->_sampleRate : 31400;
}

- (NSTimeInterval)frameInterval {
    NSTimeInterval frameInterval = (_frameInterval > 0) ? _frameInterval : 60.0;
    return frameInterval;
}

- (NSUInteger)channelCount { return 2; }

#pragma mark - Saves
-(BOOL)supportsSaveStates {
	return YES;
}

- (void)loadSaveFile:(NSString *)path forType:(int)type {
    size_t size = retro_get_memory_size(type);
    void *ramData = retro_get_memory_data(type);
    
    if (size == 0 || !ramData) {
        return;
    }
    
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data || ![data length]) {
        WLOG(@"Couldn't load save file.");
    }
    
    [data getBytes:ramData length:size];
}

- (BOOL)writeSaveFile:(NSString *)path forType:(int)type {
    size_t size = retro_get_memory_size(type);
    void *ramData = retro_get_memory_data(type);
    
    if (ramData && (size > 0)) {
        retro_serialize(ramData, size);
        NSData *data = [NSData dataWithBytes:ramData length:size];
        BOOL success = [data writeToFile:path atomically:YES];
        if (!success) {
            ELOG(@"Error writing save file");
        }
        return success;
    } else {
        return NO;
    }
}

//- (void)loadStateFromFileAtPath:(nonnull NSString *)fileName completionHandler:(nonnull void (^)(BOOL, NSError * _Nonnull __strong))block {
//    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//        NSError *error = nil;
//        BOOL success = [self loadStateFromFileAtPath:fileName error:&error];
//        block(success, error);
//    });
//}
//
//
//- (void)saveStateToFileAtPath:(nonnull NSString *)fileName completionHandler:(nonnull void (^)(BOOL, NSError * _Nonnull __strong))block {
//    // Async call the sync version
//    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//        NSError *error = nil;
//        BOOL success = [self saveStateToFileAtPath:fileName error:&error];
//        block(success, error);
//    });
//}


- (BOOL)saveStateToFileAtPath:(NSString *)path error:(NSError *__autoreleasing *)error {
    @synchronized(self) {
        size_t serial_size = retro_serialize_size();
        uint8_t *serial_data = (uint8_t *) malloc(serial_size);
        
        retro_serialize(serial_data, serial_size);
        
        NSError *error = nil;
        NSData *saveStateData = [NSData dataWithBytes:serial_data length:serial_size];
        free(serial_data);
        BOOL success = [saveStateData writeToFile:path
                                          options:NSDataWritingAtomic
                                            error:&error];
        if (!success) {
            ELOG(@"Error saving state: %@", [error localizedDescription]);
            return NO;
        }
        
        return YES;
    }
}

- (BOOL)loadStateFromFileAtPath:(NSString *)path error:(NSError *__autoreleasing *)error {
    @synchronized(self) {
        NSData *saveStateData = [NSData dataWithContentsOfFile:path];
        if (!saveStateData)
        {
            if(error != NULL) {
                NSDictionary *userInfo = @{
                                           NSLocalizedDescriptionKey: @"Failed to load save state.",
                                           NSLocalizedFailureReasonErrorKey: @"Genesis failed to read savestate data.",
                                           NSLocalizedRecoverySuggestionErrorKey: @"Check that the path is correct and file exists."
                                           };

                NSError *newError = [NSError errorWithDomain:CoreError.PVEmulatorCoreErrorDomain
                                                        code:PVEmulatorCoreErrorCodeCouldNotLoadState
                                                    userInfo:userInfo];
                *error = newError;
            }
            ELOG(@"Unable to load save state from path: %@", path);
            return NO;
        }
        
        if (!retro_unserialize([saveStateData bytes], [saveStateData length]))
        {
            if(error != NULL) {
                NSDictionary *userInfo = @{
                    NSLocalizedDescriptionKey: @"Failed to load save state.",
                    NSLocalizedFailureReasonErrorKey: @"Genesis failed to load savestate data.",
                    NSLocalizedRecoverySuggestionErrorKey: @"Check that the path is correct and file exists."
                };

                NSError *newError = [NSError errorWithDomain:CoreError.PVEmulatorCoreErrorDomain
                                                        code:PVEmulatorCoreErrorCodeCouldNotLoadState
                                                    userInfo:userInfo];
                *error = newError;
            }
            DLOG(@"Unable to load save state");
            return NO;
        }
        
        return YES;
    }
}

@dynamic supportsCheatCode;
@synthesize valueChangedHandler;

@synthesize cheatCodeTypes;


- (void)swapBuffers {
    // Does not impliment double buffering
}

- (BOOL)rendersToOpenGL {
    return NO;
}

@end

#pragma mark - Cheats

@interface PVStellaBridge (GameWithCheat) <GameWithCheat>
@end

@implementation PVStellaBridge (GameWithCheat)

- (NSArray<NSString *> *)cheatCodeTypes {
    return @[@"Game Genie", @"Pro Action Replay"];
}

-(BOOL)supportsCheatCode {
    return YES;
}

- (BOOL)setCheatWithCode:(NSString * _Nonnull)code type:(NSString * _Nonnull)type codeType:(NSString * _Nonnull)codeType cheatIndex:(uint8_t)cheatIndex enabled:(BOOL)enabled {
    // TODO: This is probably wrong @JoeMatt 5/30/24
    [self setCheat:code setType:type setEnabled:YES error:nil];
}

- (BOOL)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled  error:(NSError**)error {
    @synchronized(self) {

        BOOL cheatListSuccessfull = NO;

        NSUInteger foundIndex = [self.cheats indexOfObjectIdenticalTo:code];
        NSUInteger index = foundIndex != NSNotFound ?: self.cheats.count;

        [self.cheats insertObject:code atIndex:index];

        const char* _Nullable code_c = [code cStringUsingEncoding:NSUTF8StringEncoding];
        retro_cheat_set((unsigned int) index, enabled, code_c);

        ILOG(@"Applied Cheat Code %@ %@ %@", code, type, cheatListSuccessfull ? @"Success" : @"Failed");

        return cheatListSuccessfull;
    }
}

@end

@implementation PVStellaBridge (PV2600SystemResponderClient)
- (void)didPushPV2600Button:(PV2600Button)button forPlayer:(NSUInteger)player {
    _pad[player][A2600EmulatorValues[button]] = 1;
}

- (void)didReleasePV2600Button:(PV2600Button)button forPlayer:(NSUInteger)player {
    _pad[player][A2600EmulatorValues[button]] = 0;
}
@end
