# KIANAGY

In the middle of 2008 year, during our winter break, we (Martín Abente and José Prous) wanted to test our technical and programming skills, in order to fight boredom. Thats how we scheduled our 2 available weeks to develop a game engine from scratch, just to see what we could achieve in that very limited time.

Thats how KIANAGY (Kianagy Is Apparently Not A Game Yet) was born. Our own prototype. When we said "from scratch" we really meant it. So we decided to use the most basic dependencies, the whole engine is written in ANSI C only using standard libraries (threads and sockets) for the server, and OpenGL and SDL for the client.

## Development

The whole development process was made in a iterative way, the first version was only a few cubes with basic moves on a plane. At the end of each iteration we decided what would be the next features and what should be changed in order to accomplish them.

We worked in parallel, Martín took full responsibility for the server and José decided to work on the client side. Kianagy's architecture its pretty simple (but not necessarily good), all the virtual environment and its logic is managed on the server side, the client function is to display the environment state on a graphical way, also catching user's mouse/keyboard inputs translating it into our custom communication protocol. That means that each of us had to deal with an different domain of challenges.

## Results

Upon the final iteration, one week after the end of the winter break, we had a very decent prototype. We learn to much in those 3 weeks, in to many aspects and fields. Just to summarize kianagy's features, alias our achievements and code wall of shame :)

### Features

- Client client-server architecture, "multi-player" by design.
- Custom communication protocol, based on actions and formal definitions of "environment state".
- Custom Collision system support based on uniform spacing partitioning.
- In-map transport system, it is based on the tree-like arrangement of the environment objects with relative positions.
- Chat support
- Basic combat system
- Basic NPC (Non Player Character) support.
- NPC's behavior is modeled using "script-able" Markov chains.
- NPC-Player interactions over simply chat, based on "script-able" keywords definition.
- NPC's basic combat support.
- Custom .map support based on unofficial definitions. The parser was written using flex and bison. Its use on server side was to translate .map files to our custom uniform spacing partition binary files. On the client side for map loading.
- Custom Octree library as part of the rendering optimization.
- Custom md2 parser support, also based on unofficial definitions. Used for character model support, the code was based on a online example.
- Custom library for Cell shading rendering support.
