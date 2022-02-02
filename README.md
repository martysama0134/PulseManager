# PulseManager

This is a simple class that takes two keys. It calculates the next time to allow a specific action to be performed.

The second key has been changed from std::string to int/enum in order to improve efficiency.

### Example:

```cpp
if (!PulseManager::Instance().IncreaseClock(ch->GetPlayerID(), ePulse::GuildComment, std::chrono::milliseconds(1500))) {
    ch->ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC2(ch->GetPlayerID(), ePulse::GuildComment));
    return;
}
```
