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

const void* signal_touchesBegin     = NULL;
const void* signal_touchesEnd       = NULL;
const void* signal_touchesMove      = NULL;
const void* signal_reload           = NULL;
const void* signal_keyDown          = NULL;
const void* signal_resize           = NULL;
const void* signal_enterBackground  = NULL;
const void* signal_enterForeground  = NULL;

static void init_signals()
{
  signal_resize       = sen_signal_get_name("resize", "platform");
  signal_reload       = sen_signal_get_name("reload", "platform");
  signal_touchesBegin = sen_signal_get_name("touchesBegin", "platform");
  signal_touchesEnd   = sen_signal_get_name("touchesEnd", "platform");
  signal_touchesMove  = sen_signal_get_name("touchesMove", "platform");
  signal_keyDown      = sen_signal_get_name("keyDown", "platform");
  signal_enterBackground  = sen_signal_get_name("enterBackground", "platform");
  signal_enterForeground  = sen_signal_get_name("enterForeground", "platform");
}

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
  view.userInteractionEnabled = YES;
  self.preferredFramesPerSecond = 60;
  
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
  
  sen_textures_collect(NULL);
  sen_shaders_collect();

    // Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
}
static int is_init = 0;

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
  sen_destroy(); is_init=0;
}

#define MAX_TOUCHES 32

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  int idents [MAX_TOUCHES] = {0};
  float x[MAX_TOUCHES] = {0.0f};
  float y[MAX_TOUCHES] = {0.0f};
  int num = 0;
  for (UITouch *t in touches) {
    idents[num] = (int)t;
    x[num] = [ t locationInView:[t view]].x ;
    y[num] = [ t locationInView:[t view]].y ;
    num++;
  }
  touch_data_t td;
  td.num = num;
  td.xs = x;
  td.ys = y;
  td.ids = idents;
  
  sen_signal_emit(signal_touchesBegin, &td);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  int idents [MAX_TOUCHES] = {0};
  float x[MAX_TOUCHES] = {0.0f};
  float y[MAX_TOUCHES] = {0.0f};
  int num = 0;
  for (UITouch *t in touches) {
    idents[num] = (int)t;
    x[num] = [ t locationInView:[t view]].x ;
    y[num] = [ t locationInView:[t view]].y ;
    num++;
  }
  touch_data_t td;
  td.num = num;
  td.xs = x;
  td.ys = y;
  td.ids = idents;
  
  sen_signal_emit(signal_touchesEnd, &td);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  int idents [MAX_TOUCHES] = {0};
  float x[MAX_TOUCHES] = {0.0f};
  float y[MAX_TOUCHES] = {0.0f};
  int num = 0;
  for (UITouch *t in touches) {
    idents[num] = (int)t;
    x[num] = [ t locationInView:[t view]].x ;
    y[num] = [ t locationInView:[t view]].y ;
    num++;
  }
  touch_data_t td;
  td.num = num;
  td.xs = x;
  td.ys = y;
  td.ids = idents;
  
  sen_signal_emit(signal_touchesMove, &td);
}

#pragma mark - GLKView and GLKViewController delegate methods

static float wh[2];

- (void)update
{
  if (is_init){
    if (
        wh[0]!=self.view.bounds.size.width ||
        wh[1]!=self.view.bounds.size.height
      )
    {
      wh[0]=self.view.bounds.size.width;
      wh[1]=self.view.bounds.size.height;
      sen_signal_emit(signal_resize, wh);
    }
    
    sen_process_update();
  }
  else {
    wh[0]=self.view.bounds.size.width;
    wh[1]=self.view.bounds.size.height;
    sen_init(wh[0],wh[1]);
    init_signals();
    
    is_init=1;
  }
  
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  if (is_init) {
    sen_process_draw();
  }
}

@end
