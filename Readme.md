## Description

This code simulates communication between two clients - the sender and the recipient of the message. 
Clients are synchronized via semaphores. Also they use shared memory. 

When one of the clients dies, the second one will be closed. 
One sender and one recipient can communicate at the same time. The others are waiting. 

Functions "clean" and "check" are needed to delete  segment of shared memory and semaphores and to check that segments  with id created by keys are free

Status - development completed
