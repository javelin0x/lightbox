# ![loogo2](https://github.com/javelin0x/lightbox/assets/162154063/1948ee9d-aa36-464f-8fdc-f8d787981e25) lightbox 
is an **offline password manager** for **Windows** systems, it uses modern encryption and hashing standards (Argon2, AES256) and does a lot of what you'd want a password manager to do.
## Showcase
![lbb](https://github.com/javelin0x/lightbox/assets/162154063/622a2ae6-c5a5-4cc4-811b-7208197b6a04)

# IMPORTANT:
This repository is in BETA, there's lots to refactor, update, and change. Encryption and more security features need to be updated before it's solid for everyday use.



## How to use it?
* You can clone Lightbox locally from this Github repo and compile yourself, or grab a copy of the software from the [Releases](https://github.com/javelin0x/lightbox/releases/) tab.

- To run the program correctly, place the fonts provided on gui/font on the same directory as the .exe runs. 

- You can create a database by entering a master password and clicking on "create" on the login tab, this will create a .lbdb file named "db-" + number + ".lbdb" on the parent directory of the .exe. 

- To add credentials, click on the "+" button on the lowest row, this brings up a dialog where you can enter a username and generate a password, or enter one manually. Once finished click on "add" and the credential will be added to your database, to save changes click on the save icon, otherwise you will lose your information if any crash happens.

- Ideally, you would compile glfw by yourself, although I provide a compiled version on the [Releases](https://github.com/javelin0x/lightbox/releases/) tab.

## How do you store passwords?
Lightbox stores passwords in different forms, but the general approach is creating an encrypted file which you will decrypt when you log in with your master password corresponding to that database. Once the database is decrypted, its plaintext contents are only loaded on the application (not decrypted to disk) until you close it.

## What are the components of lightbox?
The application relies on one file at a minimum, which is your configuration file (lightbox.on), where your default DB path and other settings are stored. To log into the application you will also need a database file, which can be in multiple forms, usually as a ".lbdb" file. This file contains your encrypted credentials. 

## What metadata do you send away?
None. This password manager is built for offline use, it doesn't communicate with the internet or other programs at all.

## ⚠️Warning⚠️
Don't open other people's databases you get sent or find. Not a good idea, specially with this being on beta.

## Credits
mysterious man for the help designing the gui!

[nlohmann's JSON library](https://github.com/nlohmann)

[mrdcvlsc's AES enc/dec library](https://github.com/mrdcvlsc)

[P-H-C's argon2 hashing library](https://github.com/P-H-C/phc-winner-argon2)

[ocornut's dear imgui](https://github.com/ocornut/imgui/)
