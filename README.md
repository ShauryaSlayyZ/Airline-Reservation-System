# Airline-Reservation-System
Shaurya Agarwal (24/SE/160) |  Samyak Jain (24/SE/155)

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

AIRLINE RESERVATION SYSTEM – DSA PROJECT

Overview:
The Airline Reservation System is a Data Structures and Algorithms (DSA) based C++ project designed to automate the process of booking flights, managing passengers, and handling check-in operations. It allows both admin and passengers to perform various tasks like adding flights, booking tickets, undoing bookings, and checking in passengers efficiently using multiple data structures.

Objective:
To design and implement an efficient and persistent flight booking system using data structures for managing and retrieving data effectively. The system aims to minimize time complexity and improve performance compared to manual reservation methods.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Features:

Admin and Passenger login modes

Add, view, and manage flights

Passenger ticket booking and cancellation (undo)

Check-in queue system

Persistent storage using files

Flight search using Binary Search Tree

Sorted passenger list and binary search by ID

Route graph representation for flight connections

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Data Structures Used:

Structures (struct) – For defining entities like Flight, Booking, StackNode, QueueNode, FlightBST

Linked List – To store flights and passenger bookings dynamically

Binary Search Tree (BST) – To search and organize flights by flight ID efficiently

Queue – For managing check-in sequence (FIFO)

Stack – For undoing the last booking (LIFO)

Graph (using unordered_map and vector) – To represent flight routes (origin to destinations)

Vector (STL) – For sorting and binary searching passengers in admin operations

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Algorithms Used:

Binary Search for passenger lookup

Sorting for passenger list organization

BST insertion and traversal for flight management

Stack push/pop and Queue enqueue/dequeue operations

Linked List traversal for bookings and flights

Graph traversal to display destinations from an origin

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

File Handling:
The project uses file handling to ensure data persistence across program runs.

Flights are stored in “flights.txt”

Bookings are stored in “bookings.txt”

Uses ifstream and ofstream for reading and writing data

Data is saved automatically on exit or admin logout

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Files Used:

airline_system_combined.cpp – Main program file

flights.txt – Stores flight information

bookings.txt – Stores passenger booking information

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

How to Compile and Run:

Open a terminal or command prompt in the project directory.

Compile using:
g++ -std=c++17 airline_system_combined.cpp -o airline

Run the program:
./airline

Use “admin123” as the admin password.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Time Complexity:

Flight Search (BST): O(log n)

Booking and Cancellation (Linked List): O(n)

Check-in Queue Operations: O(1)

Sorting Passengers: O(n log n)

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Space Complexity:

Linked List and BST storage: O(n)

Queue and Stack storage: O(n)

Overall: O(n)

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

Real-World Applications:

Airline and railway ticketing systems

Online reservation and seat management systems

Transportation scheduling and logistics

Any system requiring efficient data storage, search, and retrieval
