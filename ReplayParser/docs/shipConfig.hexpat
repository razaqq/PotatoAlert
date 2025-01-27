struct CamouflageInfo
{
    u32 Id;
    u32 SchemeId;
};

struct ShipConfig
{
    u32 isSingle;
    u32 shipParamsId;
    u32 u1;
    
    // ShipConfig.dumpComponents()
    
    u32 unitsSize;  // userConfig, selected upgrades
    u32 units[unitsSize];
    
    u32 appliedExternalConfig;  // v >= 13.2.0.0
    
    // ShipConfig.dumpSlots()
    
    u32 modernizationsSize;  // modernizationSlots
    u32 modernizations[modernizationsSize];
    
    u32 signalsSize;  // exteriorSlots
    u32 signals[signalsSize];
    
    u32 signalAutoBuyInfo;
    
    u32 camoInfoSize;
    CamouflageInfo camoInfo[camoInfoSize];
    
    u32 abilitiesSize;  // abilitySlots
    u32 abilities[abilitiesSize];
    
    u32 ensignsSize;  // ensignSlots - flags
    u32 ensigns[ensignsSize];
    
    u32 ecoboostSize;  // ecoboostSlots
    u32 ecoboosts[ecoboostSize];
    
    u32 ecoboostAutoBuyInfo;  // is in a tuple with ecoboosts - ([4282380208L, 0, 4261408688L, 4250922928L], 26)
    u32 u2;
};

ShipConfig shipConfig @ 0x00;