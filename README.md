# mod-temucraft-spellbook

Server half of the Temucraft spellbook tabs: tells the Temucraft interface ([mod-temucraft-client-version](https://github.com/Temucraft/mod-temucraft-client-version),
[TemucraftSpellTabs.lua](https://github.com/Temucraft/mod-temucraft-client-version/blob/master/client/Interface/FrameXML/Temucraft/TemucraftSpellTabs.lua)) what each spell a player knows is, through addon messages (prefix TSB).

- The spellbook gets more tabs beside it: **Racials**, **Professions**, **Mounts**, **Companions** and **Guild**
  (guild perks), each a page of icons you can click or drag to your action bars.
- Spell tooltips in the spellbook say what a spell is: Racial, Profession, Guild perk, Talent, Passive, Teleport,
  Mount, Companion, or Spell.

The client asks with `LIST`; the server answers `BEGIN`, `C~id:flags,...` chunks and `END` (flags: 1 racial,
2 profession, 4 guild perk, 8 passive, 16 teleport, 32 talent, 64 mount, 128 companion). Racials are spells ranked
"Racial", guild perks "Guild Perk", professions spells of a profession skill line (and First Aid, Cooking, Fishing).

## Install

Copy the module into `modules/`, re-run CMake and build. No SQL, no config. The tabs are in the Temucraft interface
patch (client 1.2.0).
