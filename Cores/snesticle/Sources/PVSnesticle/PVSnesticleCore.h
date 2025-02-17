@import Foundation;
@import PVEmulatorCore;
@import PVCoreBridge;

@interface PVSnesticleCore : PVEmulatorCoreBridge <PVSNESSystemResponderClient>

- (void)didPushSNESButton:(PVSNESButton)button forPlayer:(NSInteger)player;
- (void)didReleaseSNESButton:(PVSNESButton)button forPlayer:(NSInteger)player;
- (void)flipBuffers;

# pragma CheatCodeSupport
- (BOOL)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled error:(NSError**)error;

@end
