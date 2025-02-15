#include <bits/stdc++.h>
#include <ctime>
#include <fstream>
using namespace std;


class TrieNode {
public:
    unordered_map<char, TrieNode*> children;
    vector<int> bookIds; 
};

class Trie {
public:
    TrieNode* root;

    Trie() {
        root = new TrieNode();
    }

    void insert(const string& key, int bookId) {
        TrieNode* current = root;
        for (char ch : key) {
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = new TrieNode();
            }
            current = current->children[ch];
        }
        current->bookIds.push_back(bookId);
    }

    vector<int> search(const string& key) {
        TrieNode* current = root;
        for (char ch : key) {
            if (current->children.find(ch) == current->children.end()) {
                return {}; // Returning empty vector if key not found
            }
            current = current->children[ch];
        }
        return current->bookIds;
    }
};

class Book {
public:
    int bookId;
    string title, author;
    bool isIssued;
    static unordered_map<int, Book> books;
    
    static unordered_map<int, pair<string, string>> issuedBooks; // {username, due date}
    static unordered_map<int, int> issuedCount;
    static unordered_map<int, vector<pair<int, string>>> bookRatings; // {rating, review}
    static Trie titleTrie; // searching books by title
    static Trie authorTrie; // searching books by author
    static unordered_map<int, pair<string, pair<string, string>>> users; // {userId,{username,{hashedPassword,role}}}

    Book(int id = 0, string t = "", string a = "", bool issued = false) : bookId(id), title(t), author(a), isIssued(issued) {}

    static void loadBooks() {
        ifstream file("books.txt");
        if (file.is_open()) {
            int id;
            string title, author;
            bool issued;
            while (file >> id >> title >> author >> issued) {
                books[id] = Book(id, title, author, issued);
                titleTrie.insert(title, id);
                authorTrie.insert(author, id);
            }
            file.close();
        }
    }

    static void saveBooks() {
        ofstream file("books.txt", ios::app);
        for (const auto& book : books) {
            file << book.second.bookId << " " << book.second.title << " " << book.second.author << " " << book.second.isIssued << "\n";
        }
        file.close();
    }

    static void loadUsers() {
        ifstream file("users.txt");
        if (file.is_open()) {
            int userId;
            string username, hashedPassword, role;
            while (file >> userId >> username >> hashedPassword >> role) {
                users[userId] = {username, {hashedPassword, role}};
            }
            file.close();
        }
    }

    /*static void saveUsers() {
        ofstream file("users.txt");
        for (const auto& user : users) {
            file << user.first << " " << user.second.first << " " << user.second.second.first << " " << user.second.second.second << "\n";
        }
        file.close();
    }*/

    static void loadRatings() {
        ifstream file("ratings.txt");
        if (file.is_open()) {
            int id, rating;
            string review;
            while (file >> id >> rating) {
                getline(file, review);
                bookRatings[id].push_back({rating, review});
            }
            file.close();
        }
    }

    static void saveRatings() {
        ofstream file("ratings.txt", ios::app);
        for (const auto& book : bookRatings) {
            for (const auto& rating : book.second) {
                file << book.first << " " << rating.first << " " << rating.second << "\n";
            }
        }
        file.close();
    }

    static void addBook();
    static void searchBook();
    static void issueBook();
    static void returnBook();
    static void calculateFine(int bookId);
    static void recommendBooks();
    static void rateBook();
    static void generateReports();
};

unordered_map<int, Book> Book::books;
unordered_map<int, pair<string, string>> Book::issuedBooks;
unordered_map<int, int> Book::issuedCount;
unordered_map<int, vector<pair<int, string>>> Book::bookRatings;
Trie Book::titleTrie;
Trie Book::authorTrie;
unordered_map<int, pair<string, pair<string, string>>> Book::users;

unordered_map<string, vector<int>> userBooks;
string currentUser;
string currentRole;


string hashPassword(const string& password) {  // hash fxn for password hashing
    hash<string> hasher;
    return to_string(hasher(password));
}

// Checking  if user is logged in
bool isLoggedIn() {
    if (currentUser.empty()) {
        cout << "Please log in first.\n";
        return false;
    }
    return true;
}

// Checking  if user is admin
bool isAdmin() {
    if (currentRole != "admin") {
        cout << "Access denied. Admin privileges required.\n";
        return false;
    }
    return true;
}

// adding book 

void Book::addBook() {
    if (!isLoggedIn() || !isAdmin()) return;

    int id;
    string title, author;
    bool issued = false;

    cout << "Enter Book ID: ";
    cin >> id;
    if (books.find(id) != books.end()) {
        cout << "Book ID already exists.\n";
        return;
    }
    cout << "Enter Book Title: ";
    cin.ignore();
    getline(cin, title);
    cout << "Enter Book Author: ";
    getline(cin, author);

    books[id] = Book(id, title, author, issued);
    titleTrie.insert(title, id);
    authorTrie.insert(author, id);
    saveBooks();
    cout << "Book added successfully!\n";
}

// Search for books by title or author
void Book::searchBook() {
    string query;
    cout << "Enter Book Title or Author: ";
    cin.ignore();
    getline(cin, query);

    vector<int> titleResults = titleTrie.search(query);
    vector<int> authorResults = authorTrie.search(query);

    if (titleResults.empty() && authorResults.empty()) {
        cout << "No books found.\n";
    } else {
        cout << "Search Results:\n";
        for (int id : titleResults) {
            cout << "ID: " << id << ", Title: " << books[id].title << ", Author: " << books[id].author << "\n";
        }
        for (int id : authorResults) {
            if (find(titleResults.begin(), titleResults.end(), id) == titleResults.end()) {
                cout << "ID: " << id << ", Title: " << books[id].title << ", Author: " << books[id].author << "\n";
            }
        }
    }
}

// Issue a book to a user
void Book::issueBook() {
    if (!isLoggedIn()) return;

    int bookId;
    cout << "Enter Book ID to issue: ";
    cin >> bookId;

    if (books.find(bookId) == books.end()) {
        cout << "Book not found.\n";
        return;
    }

    if (books[bookId].isIssued) {
        cout << "Book is already issued.\n";
        return;
    }

    // Mark book as issued
    books[bookId].isIssued = true;
    time_t now = time(0);
    tm* dueDate = localtime(&now);
    dueDate->tm_mday += 14; // Due date is 14 days from now
    mktime(dueDate);
    char buffer[11];
    strftime(buffer, 11, "%Y-%m-%d", dueDate);
    issuedBooks[bookId] = {currentUser, buffer};
    issuedCount[bookId]++;
    userBooks[currentUser].push_back(bookId);
    saveBooks();
    cout << "Book issued successfully! Due date: " << buffer << "\n";
}

// Returning book
void Book::returnBook() {
    if (!isLoggedIn()) return;

    int bookId;
    cout << "Enter Book ID to return: ";
    cin >> bookId;

    if (books.find(bookId) == books.end()) {
        cout << "Book not found.\n";
        return;
    }

    if (!books[bookId].isIssued) {
        cout << "Book is not issued.\n";
        return;
    }

    if (issuedBooks[bookId].first != currentUser) {
        cout << "You did not issue this book.\n";
        return;
    }

    // Mark book as returned
    books[bookId].isIssued = false;
    issuedBooks.erase(bookId);
    userBooks[currentUser].erase(remove(userBooks[currentUser].begin(), userBooks[currentUser].end(), bookId), userBooks[currentUser].end());
    saveBooks();
    cout << "Book returned successfully!\n";
}

//fine for overdue books
void Book::calculateFine(int bookId) {
    if (!isLoggedIn()) return;

    if (issuedBooks.find(bookId) == issuedBooks.end()) {
        cout << "Book ID " << bookId << " is not issued or does not exist.\n";
        return;
    }

    //  due date
    string dueDateStr = issuedBooks[bookId].second;

    // Parsing due date
    int dueYear, dueMonth, dueDay;
    sscanf(dueDateStr.c_str(), "%d-%d-%d", &dueYear, &dueMonth, &dueDay);

    time_t now = time(0);
    tm* currentDate = localtime(&now);
    int currentYear = 1900 + currentDate->tm_year;
    int currentMonth = 1 + currentDate->tm_mon;
    int currentDay = currentDate->tm_mday;

    // Converting due date and current date to time_t for comparison
    tm dueDate = {0, 0, 0, dueDay, dueMonth - 1, dueYear - 1900}; // tm_mon is 0-based, tm_year is years since 1900
    time_t dueTime = mktime(&dueDate);
    time_t currentTime = mktime(currentDate);

    
    double diff = difftime(currentTime, dueTime) / (60 * 60 * 24); 

    if (diff <= 0) {
        cout << "No fine. The book is not overdue.\n";
    } else {
        double finePerDay = 1.0; // $1 per day fine
        double totalFine = diff * finePerDay;
        cout << "The book is overdue by " << diff << " days. Total fine: $" << totalFine << "\n";
    }
}

// Rating & review
void Book::rateBook() {
    if (!isLoggedIn()) return;

    int id, rating;
    string review;
    cout << "Enter Book ID: ";
    cin >> id;
    if (books.find(id) == books.end()) {
        cout << "Book not found.\n";
        return;
    }

    cout << "Enter Rating (1-5): ";
    cin >> rating;
    if (rating < 1 || rating > 5) {
        cout << "Invalid rating.\n";
        return;
    }

    cout << "Enter Review (optional): ";
    cin.ignore();
    getline(cin, review);

    bookRatings[id].push_back({rating, review});
    saveRatings();
    cout << "Thank you for your feedback!\n";
}

// Recommend books based on issue count
void Book::recommendBooks() {
    if (issuedCount.empty()) {
        cout << "No books have been issued yet.\n";
        return;
    }

    vector<pair<int, int>> popularBooks(issuedCount.begin(), issuedCount.end());
    sort(popularBooks.begin(), popularBooks.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.second > b.second; // Sort by issue count in descending order
    });

    cout << "Recommended Books (Based on Popularity):\n";
    for (const auto& book : popularBooks) {
        int id = book.first;
        cout << "ID: " << id << ", Title: " << books[id].title << ", Author: " << books[id].author << ", Issues: " << book.second << "\n";
    }
}

// Generate library reports and statistics
void Book::generateReports() {
    if (!isLoggedIn() || !isAdmin()) return;

    cout << "\nLibrary Reports & Statistics:\n";

    // Most popular books
    recommendBooks();

    // Books with highest ratings
    cout << "\nBooks with Highest Ratings:\n";
    for (const auto& book : books) {
        int id = book.first;
        if (!bookRatings[id].empty()) {
            double totalRating = 0;
            for (const auto& rating : bookRatings[id]) {
                totalRating += rating.first;
            }
            double avgRating = totalRating / bookRatings[id].size();
            cout << "ID: " << id << ", Title: " << books[id].title << ", Avg Rating: " << avgRating << "\n";
        }
    }

    // Total number of books issued
    int totalIssued = 0;
    for (const auto& count : issuedCount) {
        totalIssued += count.second;
    }
    cout << "\nTotal Books Issued: " << totalIssued << "\n";

    // Total number of users
    cout << "Total Users: " << users.size() << "\n";
}

void registerUser() {
    string username, password, role;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    cout << "Enter role (admin/user): ";
    cin >> role;
    if (role != "admin" && role != "user") {
        cout << "Invalid role. Must be 'admin' or 'user'.\n";
        return;
    }

    // Counting  lines in users.txt to generate the next user ID
    ifstream inFile("users.txt");
    int lineCount = 0;
    string line;
    while (getline(inFile, line)) {
        lineCount++;
    }
    inFile.close();

    int userId = lineCount + 1; // Generating next user ID

    // Appending new user to users.txt
    ofstream outFile("users.txt", ios::app);
    outFile << userId << " " << username << " " << hashPassword(password) << " " << role << "\n";
    outFile.close();

    // Updating users map
    Book::users[userId] = {username, {hashPassword(password), role}};

    cout << "Registration successful! Your user ID is " << userId << ".\n";
}

bool loginUser() {
    int userId;
    string password;
    cout << "Enter user ID: ";
    cin >> userId;
    cout << "Enter password: ";
    cin >> password;
    if (Book::users.find(userId) == Book::users.end() || Book::users[userId].second.first != hashPassword(password)) {
        cout << "Invalid credentials!\n";
        return false;
    }
    currentUser = Book::users[userId].first;
    currentRole = Book::users[userId].second.second;
    cout << "Login successful. Welcome, " << currentUser << "!\n";
    return true;
}

void logoutUser() {
    currentUser = "";
    currentRole = "";
    cout << "Logged out successfully.\n";
}

int main() {
    Book::loadBooks();
    Book::loadUsers();
    Book::loadRatings();
    while (true) {
        cout << "\nLibrary Management System:\n";
        cout << (currentUser.empty() ? "\n" : "Welcome, " + currentUser + "!\n") << (currentUser.empty() ? "1. Register\n2. Login" : "1. Logout\n2. Add Book\n3. Search Book\n4. Issue Book\n5. Return Book\n6. Calculate Fine\n7. Rate Book\n8. Recommend Books\n9. Generate Reports\n10. Exit\n");
        int choice;
        cout<<endl;
        cout<<"Enter your choice: ";
        cin >> choice;
        switch (choice) {
            case 1: registerUser(); break;
            case 2: loginUser(); break;
            case 3: logoutUser(); break;
            case 4: Book::addBook(); break;
            case 5: Book::searchBook(); break;
            case 6: Book::issueBook(); break;
            case 7: Book::returnBook(); break;
            case 8: {
                int bookId;
                cout << "Enter Book ID to calculate fine: ";
                cin >> bookId;
                Book::calculateFine(bookId);
                break;
            }
            case 9: Book::rateBook(); break;
            case 10: Book::recommendBooks(); break;
            case 11: Book::generateReports(); break;
            case 12: return 0;
            default: cout << "Invalid choice. Try again.\n";
        }
    }
}