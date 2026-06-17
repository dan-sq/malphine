# Malphine
    c++ hobby engine by Dan Squair

# Dependencies
     C++20, cmake 4.3.2, clangd, python3.10 >=

# Usage
    1. Building the project:
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

    2. Integrating with lichess-bot:
    $ git clone https://github.com/lichess-bot-devs/lichess-bot
    $ cd lichess-bot
    Install requirements for lichess-bot. (I made a venv and installed via: $ python3 -m pip install -r requirements.txt)
    $ cp config.yml.default config.yml
    Configure config.yml, engine currently only supports uci with no options except go depth N.
    Finally, create a .env file in /malphine and add LICHESS_BOT_TOKEN="Your Token"

    3. Starting the bot:
    $ ./start.sh

# Resources
    https://www.chessprogramming.org/Main_Page
    https://github.com/lichess-bot-devs/lichess-bot/wiki
