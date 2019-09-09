


#### Multi-Threaded Webserver ###################

>>make cleanall

To compile the design : 
>> make server 

To Run the design : 
>> make run 


The server runs on port number 9000 


To test the Webserver - using the httperf use the following command. 
1. set the Server to IP address of the machine that the Webserver is running on.
2. Port no is the port that webserver is configured with. 
3. num-conn : Number of connection 
4. Rate defines the rate at which the connection are made. 

>> httperf --server=10.0.0.1 --port=9000 --num-conn=1000 --timeout=10 --rate=1000

Change the rate of requests to test for different traffic loads





