//
//  MPEAsyncViewController.h
//  MPEBouncingBalliOS
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#import <UIKit/UIKit.h>

@interface MPEAsyncViewController : UIViewController

@property (nonatomic, strong) IBOutlet UILabel * labelFOV;
@property (nonatomic, strong) IBOutlet UILabel * labelAspectRatio;
@property (nonatomic, strong) IBOutlet UILabel * labelCameraZ;

@property (nonatomic) std::function<void(float)> fovValueCallback;
@property (nonatomic) std::function<void(float)> aspectRatioValueCallback;
@property (nonatomic) std::function<void(float)> cameraZValueCallback;
@property (nonatomic) std::function<void()> resetCallback;

- (IBAction)sliderFOVChanged:(UISlider *)sender;
- (IBAction)sliderAspectRatioChanged:(UISlider *)sender;
- (IBAction)sliderCameraZChanged:(UISlider *)sender;
- (IBAction)buttonResetPressed:(id)sender;

@end
