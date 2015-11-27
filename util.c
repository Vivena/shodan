//
//  util.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include "main.h"


uint32_t itoui(int i){
    uint32_t rez= (uint32_t) i;
    if (is_bigendian()) {
        rez=htole32(rez);
    }
    return rez;
}

int uitoi(uint32_t i){
    
    if (is_bigendian()) {
        rez=le32toh(rez);
    }
    
    return (int)i;
}