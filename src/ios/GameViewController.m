//
//  GameViewController.m
//  CellMotus
//
//  Created by user on 23.12.14.
//  Copyright (c) 2014 TimeFlask. All rights reserved.
//

#import "GameViewController.h"
#import <OpenGLES/ES2/glext.h>

#include "sen.h"

//#define BUFFER_OFFSET(i) ((char *)NULL + (i))


@interface GameViewController () {
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation GameViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)dealloc
{    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
  sen_destroy();
}

#pragma mark - GLKView and GLKViewController delegate methods

static int is_init = 0;

- (void)update
{
  if (is_init)
    sen_process();
  else
    sen_init(self.view.bounds.size.width, self.view.bounds.size.height);
    
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  //  glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  sen_process();
}

@end
