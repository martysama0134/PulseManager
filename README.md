# PulseManager

This is a simple class that takes two keys. It calculates the next time to allow a specific action to be performed.

The second key can be changed from std::string to int/enum in order to improve efficiency.

### Example:

```cpp
if (!PulseManager::Instance().CheckClock(ch->GetPlayerID(), "GuildComment", std::chrono::milliseconds(1500))) {
    auto diff = PulseManager::Instance().DiffClock(ch->GetPlayerID(), "GuildComment");
    ch->ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC(diff));
    return;
}
```

Or shorter:

```cpp
if (!PulseManager::Instance().CheckClock(ch->GetPlayerID(), "GuildComment", std::chrono::milliseconds(1500))) {
    ch->ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC2(ch->GetPlayerID(), "GuildComment"));
    return;
}
```
