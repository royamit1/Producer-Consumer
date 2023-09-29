# *__News Broadcasting Simulation__*

## Table of Contents
* [Project Description](#project-description)
* [Implementation](#implementation)
* [The Use Of Threads](#the-use-of-threads)
* [Running the Project](#running-the-project)
* [Configuration File](#configuration-file)

## Project Description

This project is an implementation of a news broadcasting simulation, designed to provide experience with concurrent programming and synchronization mechanisms. The goal is to simulate the production and display of different types of news stories, while evaluating the performance impact of synchronization constructs under different loads.
![image](https://github.com/roini7/Producer-Consumer/assets/60584742/3b7d4e50-5d76-44e7-8d9f-f2715891339e)

## Implementation

#### Producer
The primary goal of the 'Producer' is to simulate the creation of news stories. Each 'Producer' is tasked with generating news stories, and their objectives include assigning a unique ID, randomly selecting a news type ('SPORTS,' 'NEWS,' 'WEATHER'), and tracking the count of stories produced. They manage private queues, which are shared with the 'Dispatcher,' to facilitate the organized distribution of news stories.

#### Dispatcher
The 'Dispatcher' holds a crucial role in efficiently managing the flow of news stories. Its key goals include distributing news stories received from 'Producers' using a Round Robin algorithm. It is responsible for sorting and categorizing stories by their respective types into distinct queues, ensuring that each story reaches the appropriate 'Co-Editor' for further processing.

#### Co-Editors
'Co-Editors' aim to refine and format the news stories for presentation. Their primary goal is to enhance the content received from the 'Dispatcher' by simulating a brief 0.1-second blocking process to represent an editing phase. Each 'Co-Editor' specializes in one news type ('SPORTS,' 'NEWS,' 'WEATHER') and contributes to the quality and consistency of the final news stories.

#### Screen-manager
The primary goal of the 'Screen-manager' is to bring the news stories to the public. It receives edited news stories from 'Co-Editors' and takes on the responsibility of displaying these stories on the screen (stdout). The 'Screen-manager' waits for the completion of all stories and the receipt of three 'DONE' messages to signal the successful conclusion of the simulation.

## The Use Of Threads
To ensure thread safety in this project, a bounded buffer mechanism is implemented. This bounded buffer plays a crucial role in managing the flow of news stories between the 'Producers,' 'Dispatcher,' 'Co-Editors,' and 'Screen-manager.' This mechanism leverages binary semaphores and counting semaphores to coordinate the concurrent execution of the actors.

#### Binary Semaphore (Mutex)
The binary semaphore (mutex) is used to protect critical sections of code where multiple threads may attempt to access shared resources simultaneously. It ensures that only one thread can access the critical section at a time, preventing race conditions.

#### Counting Semaphore
The counting semaphore is created using two binary semaphores, which is a synchronization construct commonly used for managing resources and tasks. In this project, the counting semaphore helps control the number of available slots in the bounded buffer. It regulates the flow of messages between actors.

The implementation of this 'bounded buffer' synchronization mechanism follows the principles learned in class, and it helps ensure the orderly and safe communication between the various actors in the simulation.

## Running the Project

To run the producer-consumer project, follow these steps:

1. Compile the code by running the provided `makefile`. Simply type the following command in your terminal:

   ```
   make
   ```

2. After compilation, execute the program by specifying the path to the configuration file:

   ```
   ./ex3.out path
   ```

   - Replace `path` with the actual path to your configuration file.

## Configuration File

The configuration file should follow this format:

```
<Producer ID 1>
<Number of Articles Produced by Producer 1>
<Queue Size for Producer 1>

<Producer ID 2>
<Number of Articles Produced by Producer 2>
<Queue Size for Producer 2>
...

<Final Queue Size>
```

- Each section represents a producer and specifies:
  - The producer's ID.
  - The number of articles they produce.
  - The size of the queue available to them for storing articles.

- The final section specifies the size of the final queue where edited articles are placed.

#### Example Configuration File Format

```
1
10
5

2
8
4

3
12
6

3
```

In this example, there are three producers with their respective article production counts and queue sizes, and the final queue size is 3.

Ensure there are spaces between each producer and the final queue size, as indicated above, to correctly parse the configuration file.

Feel free to modify the configuration file to match your specific requirements.




