Probably obsolete and won't be updated

# NitroSniperSquad

Maybe the most performant Nitro Sniper on GitHub.

It uses multiple tokens to seek for nitros, but all the gift requests are executed with the same one.

### This version has not been tested, as running this program would be counter the Discord TOS. You have to compile it yourself in order to use it. Please create an issue if it works.

# New

- Better information on redeemed gifts
- Support "obfuscated" gifts
- Can detect links beginning with `discord.com/gifts/<code>`, `discordapp.com/gifts/<code>`, `discord.gift/<code>`
- Handles heartbeat perfectly
- Handles reconnection for long sessions (so you don't miss any message)
- Whole code cleaning

## Build

Open `NitroSniper.pro` with Qt Creator, and click on build.

Old build releases are available at https://github.com/JacqueSatan/NitroSniperSquad/releases

## Usage

Create a `tokens.txt` file in the executable's directory. The first line is reserved for the token you want to get the nitro with, and put all the tokens you want below it. (Don't forget to copy OpenSSL binaries next to the executable)

Format:

    NDalkugfFsrsdrgDRgdrgsggrs.X
    NDrmglihsergmoiehrgmo_irgg.X
    Mzergliuhgrlhlhglsrhglsihg.X
    etc...
    
## Third party

* https://www.qt.io
* https://www.openssl.org
