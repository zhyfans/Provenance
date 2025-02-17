//
//  PVVecX+Saves.m
//  PVVecX
//
//  Created by Joseph Mattiello on 11/1/18.
//  Copyright © 2021 Provenance. All rights reserved.
//

#import "PVVecXCore+Saves.h"
#import "PVVecXCore.h"

@implementation PVVecXCoreBridge (Saves)

#pragma mark - Properties
-(BOOL)supportsSaveStates {
    return YES;
}

#pragma mark - Methods

- (BOOL)saveStateToFileAtPath:(NSString *)fileName {
    return NO;
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block {
    block(NO, nil);
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName {
    return NO;
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block {
    block(NO, nil);
}

@end
