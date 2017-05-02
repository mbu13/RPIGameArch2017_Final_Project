# Game Architecture Final Project - Matthew Bu

This final project aims to implement a simple networking feature to the existing GA engine. This project was elaborated on the fourth homework project and uses some features from the seventh assignment.

The primary features of this project include:

1) Terminal peer-to-peer chat messaging

2) Peer-to-peer movement synchronization

3) Lag simulation

Both chat messaging and synchronization utilize a UDP implementation.

## Usage

### Launch

1) Navigate to build/ga4-win64/Debug and run two instances of ga.exe.

### Startup Server and Client

2) On one of the instances, click the "Server" button on the top left to create the first entity and to start the server.

3) On the other main window, click the "Client" button on the top left to create a second entity and to start the client.

### Sending Chat

4) To send chat messages, select one of the terminal windows and directly type characters, hitting enter to send the message. You should see the message on the other terminal window.

### Moving Entities

5) To move an entity, focus on a window and use the 'i', 'j', 'k', 'l' keys. You should see the entity in the other window move as well.

### Lag Simulation

6) On the client window, click the "SIM LAG" button on the top left and increase/decrease the minimum ping constant. You should see that when moving the cube on the server window, the client's window will update the server's cube with a delay.

For more details on this project, please see this presentation:

https://github.com/mbu13/RPIGameArch2017_Final_Project/blob/master/GameArch_Final_Presentation.pdf
