#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#import "Foundation/Foundation.h"
#define MAX_BUFF 4096

#define LOG_PRINTF \
  char buffer[MAX_BUFF]; \
  va_list args; \
  va_start (args, fmt); \
  vsprintf (buffer,fmt, args); \
  va_end (args); \
  NSLog(@"%s:%s", tag,buffer);\
  FileOps *files = [[FileOps alloc] init];\
  [files WriteToStringFile:[@(buffer) mutableCopy]];

@interface FileOps : NSObject{
  NSFileManager *fileMgr;
  NSString *homeDir;
  NSString *filename;
  NSString *filepath;
}

@property(nonatomic,retain) NSFileManager *fileMgr;
@property(nonatomic,retain) NSString *homeDir;
@property(nonatomic,retain) NSString *filename;
@property(nonatomic,retain) NSString *filepath;
@end

@implementation FileOps
@synthesize fileMgr;
@synthesize homeDir;
@synthesize filename;
@synthesize filepath;


-(NSString *) setFilename{
  filename = @"cellmotus.txt";
  
  return filename;
}

/*
 Get a handle on the directory where to write and read our files. If
 it doesn't exist, it will be created.
 */

-(NSString *)GetDocumentDirectory{
  fileMgr = [NSFileManager defaultManager];
  homeDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];
  
  return homeDir;
}


/*Create a new file*/
-(void)WriteToStringFile:(NSMutableString *)textToWrite{
  filepath = [[NSString alloc] init];
  NSError *err;
  
  filepath = [self.GetDocumentDirectory stringByAppendingPathComponent:self.setFilename];
  
  BOOL ok = [textToWrite writeToFile:filepath atomically:YES encoding:NSUTF8StringEncoding  error:&err];
  
  if (!ok) {
    NSLog(@"Error writing file at %@\n%@",
          filepath, [err localizedFailureReason]);
  }
  
}
/*
 Read the contents from file
 */
-(NSString *) readFromFile
{
  filepath = [[NSString alloc] init];
  NSError *error;
  NSString *title;
  filepath = [self.GetDocumentDirectory stringByAppendingPathComponent:self.setFilename];
  NSString *txtInFile = [[NSString alloc] initWithContentsOfFile:filepath encoding:NSUnicodeStringEncoding error:&error];

  return txtInFile;
}

@end

void sen_logv(const char *tag, const char *fmt, ...) {
    LOG_PRINTF;
}

void sen_logd(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_logi(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_logw(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_loge(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}
#define tag "NSLogger"
void sen_printf(const char* fmt, ...) {
  LOG_PRINTF;
}
#undef tag