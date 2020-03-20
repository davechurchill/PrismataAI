# Prismata AI Engine
This repository contains the C++ AI Engine for [Prismata](https://store.steampowered.com/app/490220/Prismata/) (by [Lunarch Studios](http://lunarchstudios.com/)). The Prismata AI is a completely standalone project from the retail game client, and comes with an offline GUI writtten in C++/SFML that you can use to implement your own Prismata AI, or modify / play against the existing AI. 

This work is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 2.5 Canada License](https://creativecommons.org/licenses/by-nc-sa/2.5/ca/), so you can create and freely distribute your own versions of the AI for others to play against.

Getting Started with the Prismata AI:
* Watch the [GDC 2017 Prismata AI Presentation](https://youtu.be/sQSL9j7W7uA) on YouTube!
* Read the [Publications](https://github.com/davechurchill/PrismataAI/wiki/Publications) about the Prismata AI from the [AIIDE 2015 Conference](http://www.cs.mun.ca/~dchurchill/pdf/aiide15_churchill_prismata.pdf) and [Game AI Pro 3](http://www.cs.mun.ca/~dchurchill/pdf/prismata_gaip3.pdf)
* Read the [Installation Instructions](https://github.com/davechurchill/PrismataAI/wiki/Installation-Instructions) and [Tutorials](https://github.com/davechurchill/PrismataAI/wiki/GameState-Tutorial) on the [Prismata AI Wiki](https://github.com/davechurchill/PrismataAI/wiki)
* A precompiled Windows 10 version of the GUI is available for download here: [PrismataAI.zip](http://www.cs.mun.ca/~dchurchill/prismata/PrismataAI.zip)

![Prismata GUI](http://www.cs.mun.ca/~dchurchill/images/prismata_ai_gui.jpg)

In this repo you will find the following sub-projects:

* **Prismata_Engine**: C++ engine for the Prismata game rules, which allows complete simulation of Prismata games
* **Prismata_AI**: The AI code used in the retail version of Prismata (see below for details)
* **Prismata_GUI**: A fully functional SFML GUI for playing offline Prismata games vs. the AI
* **Prismata_Standalone**: Used to create a standalone executable to replace the actual Prismata client AI executable so you can play against the AI you create in the real game client
* **Prismata_Testing**: Used to benchmark, test, and run tournaments between AI agents
