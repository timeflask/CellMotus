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
  [res release];
#endif
  return ret;
  
}

void
sen_settings_set_str(const char* key, const char* value)
{
[[NSUserDefaults standardUserDefaults] setObject:[[NSString alloc] initWithUTF8String:value] forKey:[[NSString alloc] initWithUTF8String:key]];
[[NSUserDefaults standardUserDefaults] synchronize];
}

