// airline_system_combined.cpp
// Compile: g++ -std=c++17 airline_system_combined.cpp -o airline
// Run: ./airline
//
// Combined, efficient, persistent Airline Reservation System
// Features:
//  - Flights stored as single linked-list nodes
//  - Flights indexed in a BST (search by id)
//  - Flight route graph (origin -> list of destinations)
//  - Bookings as linked list
//  - Undo stack (last booking)
//  - Check-in queue
//  - Admin utilities: sorted passenger list, binary search by passenger ID
//  - File persistence: flights.txt, bookings.txt

#include <bits/stdc++.h>
using namespace std;

// ----------------------------- Data Structures -----------------------------
struct Flight {
    int id;
    string origin;
    string destination;
    string terminal;
    string time;
    int seats;
    double price;
    Flight* next;
    Flight(int _id=0) : id(_id), seats(0), price(0.0), next(nullptr) {}
};

struct Booking {
    int passengerId;
    string passenger;
    string gender;
    int age;
    int flightId;
    int seats;
    Booking* next;
    Booking(): passengerId(0), age(0), flightId(0), seats(0), next(nullptr) {}
};

struct StackNode {           // Undo stack
    Booking booking;
    StackNode* next;
    StackNode(const Booking& b, StackNode* n=nullptr) : booking(b), next(n) {}
};

struct QueueNode {           // Check-in queue
    Booking booking;
    QueueNode* next;
    QueueNode(const Booking& b) : booking(b), next(nullptr) {}
};

struct FlightBST {           // BST node referencing Flight*
    Flight* flight;
    FlightBST* left;
    FlightBST* right;
    FlightBST(Flight* f=nullptr) : flight(f), left(nullptr), right(nullptr) {}
};

// ----------------------------- Global State -----------------------------
Flight* flightsHead = nullptr;      // single linked list of flights
FlightBST* flightRoot = nullptr;    // BST index on flight id
unordered_map<string, vector<string>> routeGraph; // origin -> destinations
Booking* bookingsHead = nullptr;    // linked list of bookings
StackNode* undoTop = nullptr;       // stack top for undo
QueueNode* qFront = nullptr;        // check-in queue front
QueueNode* qRear = nullptr;         // check-in queue rear
int nextPassengerId = 1001;

// ----------------------------- File Names -----------------------------
const string FLIGHTS_FILE = "flights.txt";
const string BOOKINGS_FILE = "bookings.txt";

// ----------------------------- Utility Helpers -----------------------------
void safeGetline(string &out) {
    getline(cin, out);
    if(!out.empty() && out.back()=='\r') out.pop_back(); // handle CRLF if any
}

// ----------------------------- Flight List / BST / Graph -----------------------------
void insertFlightToLinkedList(Flight* f) {
    if(!flightsHead) { flightsHead = f; return; }
    Flight* t = flightsHead;
    while(t->next) t = t->next;
    t->next = f;
}

void insertFlightToBST(Flight* f) {
    FlightBST* node = new FlightBST(f);
    if(!flightRoot) { flightRoot = node; return; }
    FlightBST* cur = flightRoot;
    FlightBST* parent = nullptr;
    while(cur) {
        parent = cur;
        if(f->id < cur->flight->id) cur = cur->left;
        else cur = cur->right;
    }
    if(f->id < parent->flight->id) parent->left = node;
    else parent->right = node;
}

void addRoute(const string& from, const string& to) {
    auto &vec = routeGraph[from];
    // prevent duplicate destinations
    if(find(vec.begin(), vec.end(), to) == vec.end()) vec.push_back(to);
}

Flight* findFlightByIdBST(FlightBST* root, int id) {
    if(!root) return nullptr;
    if(root->flight->id == id) return root->flight;
    if(id < root->flight->id) return findFlightByIdBST(root->left, id);
    return findFlightByIdBST(root->right, id);
}

Flight* findFlightLinear(int id) {
    Flight* t = flightsHead;
    while(t) {
        if(t->id == id) return t;
        t = t->next;
    }
    return nullptr;
}

void viewFlights() {
    if(!flightsHead) { cout << "No flights available.\n"; return; }
    cout << "\n--- Flight List ---\n";
    Flight* t = flightsHead;
    while(t) {
        cout << "ID: " << t->id << " | " << t->origin << " -> " << t->destination
             << " | Terminal: " << t->terminal << " | Time: " << t->time
             << " | Seats: " << t->seats << " | Price: " << t->price << "\n";
        t = t->next;
    }
}

void viewDestinationsFrom(const string& origin) {
    auto it = routeGraph.find(origin);
    if(it == routeGraph.end() || it->second.empty()) {
        cout << "No flights from " << origin << "\n";
        return;
    }
    cout << "Flights from " << origin << " go to:\n";
    for(const auto &d : it->second) cout << "- " << d << "\n";
}

// ----------------------------- Booking List / Queue / Stack -----------------------------
void addBookingToLinkedList(const Booking &b) {
    Booking* nb = new Booking();
    *nb = b; nb->next = nullptr;
    if(!bookingsHead) bookingsHead = nb;
    else {
        Booking* t = bookingsHead;
        while(t->next) t = t->next;
        t->next = nb;
    }
}

bool removeBookingFromListById(int pid) {
    if(!bookingsHead) return false;
    Booking* cur = bookingsHead;
    Booking* prev = nullptr;
    while(cur) {
        if(cur->passengerId == pid) {
            if(!prev) bookingsHead = cur->next;
            else prev->next = cur->next;
            delete cur;
            return true;
        }
        prev = cur; cur = cur->next;
    }
    return false;
}

void pushUndo(const Booking &b) {
    undoTop = new StackNode(b, undoTop);
}

bool popUndo(Booking &out) {
    if(!undoTop) return false;
    out = undoTop->booking;
    StackNode* temp = undoTop;
    undoTop = undoTop->next;
    delete temp;
    return true;
}

void enqueueCheckin(const Booking &b) {
    QueueNode* node = new QueueNode(b);
    if(!qRear) { qFront = qRear = node; return; }
    qRear->next = node; qRear = node;
}

bool removeFromQueueById(int pid) {
    if(!qFront) return false;
    QueueNode* cur = qFront; QueueNode* prev = nullptr;
    while(cur) {
        if(cur->booking.passengerId == pid) {
            if(!prev) { qFront = cur->next; if(!qFront) qRear = nullptr; }
            else {
                prev->next = cur->next;
                if(cur == qRear) qRear = prev;
            }
            delete cur;
            return true;
        }
        prev = cur; cur = cur->next;
    }
    return false;
}

void dequeueCheckin() {
    if(!qFront) { cout << "No passengers in queue.\n"; return; }
    QueueNode* temp = qFront;
    cout << "\n✅ Passenger Checked-in: " << temp->booking.passenger
         << " | ID: " << temp->booking.passengerId
         << " | Flight: " << temp->booking.flightId << "\n";
    qFront = qFront->next;
    if(!qFront) qRear = nullptr;
    delete temp;
}

void viewCheckinQueue() {
    if(!qFront) { cout << "No passengers waiting for check-in.\n"; return; }
    cout << "\n--- Check-in Queue ---\n";
    QueueNode* t = qFront;
    while(t) {
        cout << "ID: " << t->booking.passengerId << " | " << t->booking.passenger
             << " | Flight: " << t->booking.flightId << "\n";
        t = t->next;
    }
}

// ----------------------------- File Persistence -----------------------------
void saveFlightsToFile() {
    ofstream fout(FLIGHTS_FILE);
    Flight* t = flightsHead;
    while(t) {
        // CSV: id,origin,destination,terminal,time,seats,price
        fout << t->id << "," << t->origin << "," << t->destination << ","
             << t->terminal << "," << t->time << "," << t->seats << "," << t->price << "\n";
        t = t->next;
    }
    fout.close();
}

void saveBookingsToFile() {
    ofstream fout(BOOKINGS_FILE);
    Booking* t = bookingsHead;
    while(t) {
        // CSV: pid,passenger,gender,age,flightId,seats
        fout << t->passengerId << "," << t->passenger << "," << t->gender << ","
             << t->age << "," << t->flightId << "," << t->seats << "\n";
        t = t->next;
    }
    fout.close();
}

void loadFlightsFromFile() {
    ifstream fin(FLIGHTS_FILE);
    if(!fin.is_open()) return;
    string line;
    while(getline(fin, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        string token;
        Flight* f = new Flight();
        getline(ss, token, ','); f->id = stoi(token);
        getline(ss, f->origin, ',');
        getline(ss, f->destination, ',');
        getline(ss, f->terminal, ',');
        getline(ss, f->time, ',');
        getline(ss, token, ','); f->seats = stoi(token);
        getline(ss, token, ','); f->price = stod(token);
        f->next = nullptr;
        insertFlightToLinkedList(f);
        insertFlightToBST(f);
        addRoute(f->origin, f->destination);
    }
    fin.close();
}

void loadBookingsFromFile() {
    ifstream fin(BOOKINGS_FILE);
    if(!fin.is_open()) return;
    string line;
    while(getline(fin, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        Booking b;
        string token;
        getline(ss, token, ','); b.passengerId = stoi(token);
        getline(ss, b.passenger, ',');
        getline(ss, b.gender, ',');
        getline(ss, token, ','); b.age = stoi(token);
        getline(ss, token, ','); b.flightId = stoi(token);
        getline(ss, token, ','); b.seats = stoi(token);
        b.next = nullptr;
        // add to booking list & queue & undo
        addBookingToLinkedList(b);
        pushUndo(b);
        enqueueCheckin(b);
        nextPassengerId = max(nextPassengerId, b.passengerId + 1);
    }
    fin.close();
}

// ----------------------------- Booking Workflow -----------------------------
void bookTicketInteractive() {
    int fid; cout << "\nEnter Flight ID: "; cin >> fid;
    Flight* f = findFlightByIdBST(flightRoot, fid);
    if(!f) {
        cout << "Flight not found (BST). Trying linear search... \n";
        f = findFlightLinear(fid);
        if(!f) { cout << "Flight not found.\n"; return; }
    }
    string name; string gender; int age; int seats;
    cout << "Enter name: "; cin >> ws; safeGetline(name);
    cout << "Enter gender (M/F): "; cin >> gender;
    cout << "Enter age: "; cin >> age;
    cout << "Enter seats: "; cin >> seats;
    if(seats <= 0) { cout << "Invalid seat count.\n"; return; }
    if(seats > f->seats) { cout << "Not enough seats available.\n"; return; }
    f->seats -= seats;
    Booking b;
    b.passengerId = nextPassengerId++;
    b.passenger = name;
    b.gender = gender;
    b.age = age;
    b.flightId = fid;
    b.seats = seats;
    b.next = nullptr;
    addBookingToLinkedList(b);
    pushUndo(b);
    enqueueCheckin(b);
    cout << "\n✅ Booking successful! Passenger ID: " << b.passengerId << "\n";
    cout << "Flight: " << f->origin << " -> " << f->destination << " | Time: " << f->time << " | Terminal: " << f->terminal << "\n";
    cout << "Total price: " << (f->price * seats) << "\n";
}

void undoLastBookingInteractive() {
    Booking last;
    if(!popUndo(last)) { cout << "Nothing to undo.\n"; return; }
    // restore seats
    Flight* f = findFlightByIdBST(flightRoot, last.flightId);
    if(f) f->seats += last.seats;
    bool removedList = removeBookingFromListById(last.passengerId);
    bool removedQueue = removeFromQueueById(last.passengerId);
    cout << "Undo completed for passenger: " << last.passenger << " (ID: " << last.passengerId << ")\n";
    if(removedList) cout << "- Removed from bookings list.\n";
    if(removedQueue) cout << "- Removed from check-in queue.\n";
}

// ----------------------------- Admin Utilities -----------------------------
vector<Booking> getAllBookingsVector() {
    vector<Booking> out;
    Booking* t = bookingsHead;
    while(t) {
        Booking copy = *t; copy.next = nullptr;
        out.push_back(copy);
        t = t->next;
    }
    return out;
}

void viewAllPassengersSorted() {
    vector<Booking> list = getAllBookingsVector();
    if(list.empty()) { cout << "No bookings yet.\n"; return; }
    sort(list.begin(), list.end(), [](const Booking &a, const Booking &b){
        return a.passengerId < b.passengerId;
    });
    cout << "\n--- Passengers (sorted by ID, low ID = high priority) ---\n";
    for(const auto &b : list) {
        cout << "ID: " << b.passengerId << " | " << b.passenger
             << " | Age: " << b.age << " | Flight: " << b.flightId
             << " | Seats: " << b.seats << "\n";
    }
}

int binarySearchPassengerId(const vector<Booking>& list, int pid) {
    int l = 0, r = (int)list.size() - 1;
    while(l <= r) {
        int m = l + (r - l)/2;
        if(list[m].passengerId == pid) return m;
        else if(list[m].passengerId < pid) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

void adminSearchPassengerById() {
    vector<Booking> list = getAllBookingsVector();
    if(list.empty()) { cout << "No bookings yet.\n"; return; }
    sort(list.begin(), list.end(), [](const Booking &a, const Booking &b){
        return a.passengerId < b.passengerId;
    });
    int pid; cout << "Enter Passenger ID to search: "; cin >> pid;
    int idx = binarySearchPassengerId(list, pid);
    if(idx == -1) { cout << "Passenger not found.\n"; return; }
    const Booking &b = list[idx];
    cout << "\n--- Booking Found ---\n";
    cout << "ID: " << b.passengerId << " | Name: " << b.passenger << " | Age: " << b.age
         << " | Flight: " << b.flightId << " | Seats: " << b.seats << " | Gender: " << b.gender << "\n";
    Flight* f = findFlightByIdBST(flightRoot, b.flightId);
    if(f) cout << "Route: " << f->origin << " -> " << f->destination << " | Time: " << f->time << "\n";
}

// ----------------------------- Menus -----------------------------
void adminMenu() {
    while(true) {
        cout << "\n===== ADMIN MENU =====\n";
        cout << "1. Add Flight\n2. View Flights\n3. View Route Destinations\n4. View Check-in Queue\n5. Check-in Passenger \n6. View All Passengers \n7. Search Passenger by ID\n8. Save & Logout\nChoice: ";
        int ch; cin >> ch;
        if(ch == 1) {
            int id, seats; double price; string from, to, term, time;
            cout << "Enter Flight ID: "; cin >> id;
            cout << "Enter Origin: "; cin >> ws; safeGetline(from);
            cout << "Enter Destination: "; safeGetline(to);
            cout << "Enter Terminal: "; safeGetline(term);
            cout << "Enter Departure Time: "; safeGetline(time);
            cout << "Enter Total Seats: "; cin >> seats;
            cout << "Enter Price per Seat: "; cin >> price;
            Flight* f = new Flight();
            f->id = id; f->origin = from; f->destination = to; f->terminal = term;
            f->time = time; f->seats = seats; f->price = price; f->next = nullptr;
            insertFlightToLinkedList(f);
            insertFlightToBST(f);
            addRoute(from, to);
            cout << "✅ Flight added.\n";
        } else if(ch==2) viewFlights();
        else if(ch==3) { string org; cout << "Origin: "; cin >> ws; safeGetline(org); viewDestinationsFrom(org); }
        else if(ch==4) viewCheckinQueue();
        else if(ch==5) dequeueCheckin();
        else if(ch==6) viewAllPassengersSorted();
        else if(ch==7) adminSearchPassengerById();
        else if(ch==8) { saveFlightsToFile(); saveBookingsToFile(); cout << "Saved. Logging out.\n"; break; }
        else cout << "Invalid choice.\n";
    }
}

void passengerMenu() {
    while (true) {
        cout << "\n===== PASSENGER MENU =====\n";
        cout << "1. View Flights\n2. Book Ticket\n3. Undo Last Booking\n4. Check My Reservation\n5. Exit to Main\nChoice: ";
        int ch;
        cin >> ch;

        if (ch == 1) {
            viewFlights();
        } 
        else if (ch == 2) {
            bookTicketInteractive();
        } 
        else if (ch == 3) {
            undoLastBookingInteractive();
        } 
        else if (ch == 4) {
            int pid;
            cout << "Enter Passenger ID: ";
            cin >> pid;
            Booking* t = bookingsHead;
            bool found = false;

            while (t) {
                if (t->passengerId == pid) {
                    cout << "\n--- Reservation Details ---\n";
                    cout << "Passenger ID: " << t->passengerId
                         << " | Name: " << t->passenger
                         << " | Gender: " << t->gender
                         << " | Age: " << t->age
                         << " | Flight ID: " << t->flightId
                         << " | Seats: " << t->seats << "\n";

                    Flight* f = findFlightByIdBST(flightRoot, t->flightId);
                    if (f) {
                        cout << "Route: " << f->origin << " -> " << f->destination
                             << " | Time: " << f->time
                             << " | Terminal: " << f->terminal
                             << " | Price per seat: " << f->price << "\n";
                    }
                    found = true;
                    break;
                }
                t = t->next;
            }

            if (!found) cout << "No reservation found for Passenger ID " << pid << "\n";
        } 
        else if (ch == 5) {
            break;
        } 
        else {
            cout << "Invalid choice.\n";
        }
    }
}


// ----------------------------- Cleanup -----------------------------
void freeAllMemory() {
    // flights
    while(flightsHead) {
        Flight* t = flightsHead; flightsHead = flightsHead->next; delete t;
    }
    // BST nodes
    function<void(FlightBST*)> freeBST = [&](FlightBST* node){
        if(!node) return;
        freeBST(node->left); freeBST(node->right); delete node;
    };
    freeBST(flightRoot); flightRoot = nullptr;
    // bookings
    while(bookingsHead) {
        Booking* t = bookingsHead; bookingsHead = bookingsHead->next; delete t;
    }
    // stack
    while(undoTop) {
        StackNode* t = undoTop; undoTop = undoTop->next; delete t;
    }
    // queue
    while(qFront) { QueueNode* t = qFront; qFront = qFront->next; delete t; }
    routeGraph.clear();
}

// ----------------------------- main -----------------------------
int main() {
    loadFlightsFromFile();
    loadBookingsFromFile();

    cout << "===== AIRLINE RESERVATION SYSTEM =====\n";
    while(true) {
        cout << "\n1. Login as Admin\n2. Login as Passenger\n3. Exit\nChoice: ";
        int choice; cin >> choice;
        if(choice == 1) {
            string pwd; cout << "Enter admin password: "; cin >> pwd;
            if(pwd == "admin123") adminMenu();
            else cout << "Wrong password.\n";
        } else if(choice == 2) passengerMenu();
        else if(choice == 3) {
            saveFlightsToFile();
            saveBookingsToFile();
            cout << "Saved and exiting. Goodbye!\n";
            break;
        } else cout << "Invalid option.\n";
    }

    freeAllMemory();
    return 0;
}
