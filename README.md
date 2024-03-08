# NEC via UART

This is a stange project for a paticular issue I face because I'm lazy.

I own a 5.1 Sound System which is connected to my TV via  S/PDIF (also called Tosslink). Because of this I cant use the Volume Buttons on my Remote to change the volume. The 5.1 Sound System comes with its own remote to change volume amoung other things. But as the IR-Receiver on the Subwoofer is in not reachable from the Place where the IR Receiver is. I could get up walk a few meters than change the volume and sit back down, but I don't want to. So I figured, lets do a relay.

An Seeeduino XIAO SAMD21 will reveive the IR-Signal. Serializes the NEC-Protocol into a JSON-String. Transmit this JSON-String to an ESP32 via UART. The ESP-32 deserializes the JSON-String back into NEC and sends it via IR to the 5.1 Sound-System. I considered creating a Byte-Protocol insted of using Json, but as the amount of data I send and the additional work this would endure, I don't see the point in doing so. Although I creaded a Byte-Protocol defintion inside a private repo, but at the moment there is no implermentation.

But well now I face another issue. How to I konw if the operation was successful and read out the Volume, Channel-Levels and whatever.

Well and as AI was kind of the Hype last year and this will likly continue into 2024. Let's put sume AI into it. The ESP-32 is intended to use machine vision to read the four digits or letters on the 7-Segment display and transmit them back to the Seeeduino XIAO SAMD21. Which then displays them on another four digit 7 segment display.

As the Seeeduino XIAO SAMD21 does not have a lot of pins, and the one I use has even 3 broken pins I use an MCP23S17 SPI-Port-Expander to control the four digit 7 segment display.

Well thats the plan at least let's see how it is going.
