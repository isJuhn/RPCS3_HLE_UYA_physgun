# RPCS3_HLE_UYA_physgun

RPCS3 Physgun mod for Ratchet & Clank 3 (BCES01503, NPEA00387) in dll form. Use my [HLE branch](https://github.com/isJuhn/rpcs3/tree/HLE) to load it.

[Showcase video](https://www.youtube.com/watch?v=5Byg8dUcqoc)

patch.yml:
```
PPU-2b534405de98e28b1368947c8f14d9a386bc53de: #BCES01503 R&C 3
  - [dll, 0, "RPCS3_HLE_UYA_physgun.dll"]
  - [func, 0x5A13C8, "physgun_tick"]
  - [bef32, 0x5F0D6C, 0]
  
PPU-25754a689f16c96ab071a738b8e46e4e2e12b327: #NPEA00387 R&C 3
  - [dll, 0, "RPCS3_HLE_UYA_physgun.dll"]
  - [func, 0x5A0F88, "physgun_tick"]
  - [bef32, 0x5F092C, 0]
```
