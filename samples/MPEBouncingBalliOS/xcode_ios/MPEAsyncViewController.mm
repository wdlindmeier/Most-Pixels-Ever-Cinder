//
//  MPEAsyncViewController.mm
//  MPEBouncingBalliOS
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#import "MPEAsyncViewController.h"

@interface MPEAsyncViewController ()

@end

@implementation MPEAsyncViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (IBAction)sliderFOVChanged:(UISlider *)sender
{
    float newValue = 1.0f + (sender.value * 179.0f);
    if (_fovValueCallback)
    {
        _fovValueCallback(newValue);
    }
    self.labelFOV.text = [NSString stringWithFormat:@"%0.2f", newValue];
}

- (IBAction)sliderAspectRatioChanged:(UISlider *)sender
{
    float newValue = 0.f + (sender.value * 2.0f);
    if (_aspectRatioValueCallback)
    {
        _aspectRatioValueCallback(newValue);
    }
    self.labelAspectRatio.text = [NSString stringWithFormat:@"%0.2f", newValue];
}

- (IBAction)sliderCameraZChanged:(UISlider *)sender
{
    float newValue = -1500.0f + (sender.value * 1500.0f);
    if (_cameraZValueCallback)
    {
        _cameraZValueCallback(newValue);
    }
    self.labelCameraZ.text = [NSString stringWithFormat:@"%0.2f", newValue];
}

- (IBAction)buttonResetPressed:(id)sender
{
    if (_resetCallback)
    {
        _resetCallback();
    }
}

@end
