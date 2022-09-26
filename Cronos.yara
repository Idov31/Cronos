/*
   Author: Ido Veltzman
   Date: 2022-09-26
   Identifier: Cronos
   Reference: https://github.com/Idov31/Cronos
*/

rule Cronos {
   meta:
      description = "Cronos Sleep Obfuscation"
      author = "Ido Veltzman"
      reference = "https://github.com/Idov31/Cronos"
      date = "2022-09-26"
   strings:
      $s1 = "[ + ] Code executed!" fullword ascii
      $s2 = "[ - ] Failed to create waitable timers: %d" fullword ascii
      $s3 = "[ - ] Failed to SetWaitableTimer: %d" fullword ascii
      $s4 = "[!] Error finding gadget" fullword ascii
      $s5 = "[ - ] Failed to capture context: %d" fullword ascii
      $s6 = "EncryptionTimer" fullword wide
      $s7 = "[ + ] Sleeping" fullword ascii
      $s8 = "SystemFunction032" fullword ascii
      $s9 = "NtContinue" fullword ascii
      $s10 = "DecryptionTimer" fullword wide
      $s11 = "ProtectionRWTimer" fullword wide
      $s12 = "ProtectionRWXTimer" fullword wide
      $s13 = "DummyTimer" fullword wide

      $op0 = { 41 52 41 51 41 ba ff ff ff ff 41 52 51 41 ba 01 }
      $op1 = { b9 ff ff ff ff ff 15 07 5c 01 00 41 b8 d0 04 00 }
      $op2 = { e8 c4 ff ff ff 33 d2 48 8d 4d f0 41 b8 d0 04 00 }
   condition:
      uint16(0) == 0x5a4d and filesize < 400KB and
      ( 8 of them and all of ($op*) )
}
