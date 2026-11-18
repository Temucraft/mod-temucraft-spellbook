/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

// Spellbook data for the Temucraft interface (addon messages, prefix TSB): what each known spell is, so the
// spellbook can show Racials, Professions and Guild tabs and label spells (passive, teleport, talent...).
// The client asks with "LIST"; the server answers "BEGIN", then "C~id:flags,id:flags,..." chunks, then "END".
// flags: 1 racial, 2 profession, 4 guild perk, 8 passive, 16 teleport, 32 talent, 64 mount, 128 companion.

#include "DBCStores.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemucraftAddonMessage.h"
#include <cstring>
#include <string>

namespace
{
    constexpr char const* PREFIX = "TSB";

    enum Flag : uint32
    {
        F_RACIAL = 1, F_PROFESSION = 2, F_GUILD = 4, F_PASSIVE = 8, F_TELEPORT = 16, F_TALENT = 32, F_MOUNT = 64, F_COMPANION = 128
    };

    bool RankStarts(SpellInfo const* info, char const* text)
    {
        char const* rank = info->Rank[LOCALE_enUS];
        return rank && !std::strncmp(rank, text, std::strlen(text));
    }

    uint32 Flags(SpellInfo const* info)
    {
        uint32 f = 0;
        if (RankStarts(info, "Racial"))
            f |= F_RACIAL;
        if (RankStarts(info, "Guild Perk"))
            f |= F_GUILD;
        if (info->IsPassive())
            f |= F_PASSIVE;
        if (GetTalentSpellPos(info->Id))
            f |= F_TALENT;
        for (SpellEffectInfo const& e : info->GetEffects())
        {
            if (e.Effect == SPELL_EFFECT_TELEPORT_UNITS || e.Effect == SPELL_EFFECT_TRIGGER_SPELL_2 /* portals */)
                f |= F_TELEPORT;
            if (e.ApplyAuraName == SPELL_AURA_MOUNTED)
                f |= F_MOUNT;
            if (e.Effect == SPELL_EFFECT_SUMMON && e.MiscValueB == 3221 /* companion */)
                f |= F_COMPANION;
        }
        SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(info->Id);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
            if (SkillLineEntry const* line = sSkillLineStore.LookupEntry(itr->second->SkillLine))
            {
                if (line->categoryId == SKILL_CATEGORY_PROFESSION ||
                    line->id == SKILL_FIRST_AID || line->id == SKILL_COOKING || line->id == SKILL_FISHING)
                    f |= F_PROFESSION;
                if (line->id == SKILL_MOUNTS)
                    f |= F_MOUNT;
                if (line->id == SKILL_COMPANIONS)
                    f |= F_COMPANION;
            }
        return f;
    }

    void SendList(Player* player)
    {
        TemucraftAddonSend(player, PREFIX, "BEGIN");
        std::string chunk = "C~";
        for (auto const& [id, spell] : player->GetSpellMap())
        {
            if (spell->State == PLAYERSPELL_REMOVED || !spell->Active)
                continue;
            SpellInfo const* info = sSpellMgr->GetSpellInfo(id);
            if (!info)
                continue;
            uint32 const f = Flags(info);
            if (!f)
                continue;
            std::string item = std::to_string(id) + ":" + std::to_string(f) + ",";
            if (chunk.size() + item.size() > 240)
            {
                TemucraftAddonSend(player, PREFIX, chunk);
                chunk = "C~";
            }
            chunk += item;
        }
        if (chunk.size() > 2)
            TemucraftAddonSend(player, PREFIX, chunk);
        TemucraftAddonSend(player, PREFIX, "END");
    }
}

class TemucraftSpellbookPlayer : public PlayerScript
{
public:
    TemucraftSpellbookPlayer() : PlayerScript("TemucraftSpellbookPlayer", { PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT }) { }

    using PlayerScript::OnPlayerCanUseChat;
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* receiver) override
    {
        std::string body;
        if (!TemucraftAddonParse(player, type, language, msg, receiver, PREFIX, body))
            return true;
        if (body == "LIST")
            SendList(player);
        return false;
    }
};

void AddTemucraftSpellbookScripts()
{
    new TemucraftSpellbookPlayer();
}
