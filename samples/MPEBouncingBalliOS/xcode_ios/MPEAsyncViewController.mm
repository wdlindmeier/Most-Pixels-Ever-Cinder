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
    self.sliderCameraZ.value = kInitialCameraZ;
    self.sliderAspectRatio.value = kInitialAspectRatio;
    self.sliderFOV.value = kInitialFOV;
    [self updateLabels];
}

- (void)updateLabels
{
    self.labelFOV.text = [NSString stringWithFormat:@"%0.2f", self.sliderFOV.value];
    self.labelAspectRatio.text = [NSString stringWithFormat:@"%0.2f", self.sliderAspectRatio.value];
    self.labelCameraZ.text = [NSString stringWithFormat:@"%0.2f", self.sliderCameraZ.value];
}

- (IBAction)sliderFOVChanged:(UISlider *)sender
{
    if (_fovValueCallback)
    {
        _fovValueCallback(sender.value);
    }
    [self updateLabels];
}

- (IBAction)sliderAspectRatioChanged:(UISlider *)sender
{
    if (_aspectRatioValueCallback)
    {
        _aspectRatioValueCallback(sender.value);
    }
    [self updateLabels];
}

- (IBAction)sliderCameraZChanged:(UISlider *)sender
{
    if (_cameraZValueCallback)
    {
        _cameraZValueCallback(sender.value);
    }
    [self updateLabels];
}

- (IBAction)buttonResetPressed:(id)sender
{
    if (_resetCallback)
    {
        _resetCallback();
    }
}

@end
