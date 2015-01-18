#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>

#import "Foundation/Foundation.h"
#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Settings"

static NSString *get_string(NSUserDefaults *dict, id key, NSString *fallback) {
    id result = [dict objectForKey: key];

    if (!result)
        result = fallback;
    else if (![result isKindOfClass: [NSString class]])
        result = [result description];

    return result;
}

char*
sen_settings_get_str(const char* key, const char* defaultValue)
{
  char *ret;
  NSString *res = get_string( [NSUserDefaults standardUserDefaults],
                             [[NSString alloc] initWithUTF8String:key],
                             [[NSString alloc] initWithUTF8String:defaultValue]);
  ret = res ? sen_strdup ( [res UTF8String] ) : sen_strdup(defaultValue);
#if SEN_PLATFORM==SEN_PLATFORM_MAC
//  [res release];
#endif
  return ret;
  
}
#if SEN_PLATFORM==SEN_PLATFORM_IOS
extern void __sen_ios_switch_screen_on (int isKeepScreen);
#endif
void
sen_settings_set_str(const char* key, const char* value)
{
#if SEN_PLATFORM==SEN_PLATFORM_IOS
  __sen_ios_switch_screen_on (value && strstr(value, "keep_screen_on = true"));
#endif
[[NSUserDefaults standardUserDefaults] setObject:[[NSString alloc] initWithUTF8String:value] forKey:[[NSString alloc] initWithUTF8String:key]];
[[NSUserDefaults standardUserDefaults] synchronize];
}

