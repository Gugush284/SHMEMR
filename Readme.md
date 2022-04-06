## Description
This code simulates communication between two clients - the sender and the recipient of the message. 
Clients are synchronized via semaphores. When one of the clients dies, the second one will be closed. 
One sender and one recipient can communicate at the same time. The others are waiting
