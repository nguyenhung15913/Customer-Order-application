/*
Name: Hung Nguyen
ID: 101984185
SECTION: DBS311NHH
Date: 12/11/2020
Assignment 2
*/

/*
PL/SQL

SET SERVEROUTPUT ON;

CREATE OR REPLACE PROCEDURE find_customer (customerId IN NUMBER, found OUT NUMBER)
AS
ok NUMBER;
BEGIN
    found := 1;
    SELECT customer_id INTO ok FROM customers WHERE customer_id = customerId;

    EXCEPTION
    WHEN no_data_found THEN
        found := 0;
END;

CREATE OR REPLACE PROCEDURE find_product (productId IN NUMBER, price OUT products.list_price%TYPE)
AS
BEGIN
    SELECT list_price INTO price FROM products WHERE product_id = productId;

    EXCEPTION
    WHEN no_data_found THEN
        price := 0;
END;

CREATE OR REPLACE PROCEDURE add_order (customerId IN NUMBER, new_order_id OUT NUMBER)
AS
BEGIN
    SELECT max(order_id) INTO new_order_id FROM orders;
    new_order_id := new_order_id + 1;
    INSERT INTO orders
      VALUES(new_order_id, customerId, 'Shipped', 56, sysdate);
END;

CREATE OR REPLACE PROCEDURE add_order_item (orderId IN order_items.order_id%type,
                                itemId IN order_items.item_id%type,
            productId IN order_items.product_id%type,
            quant IN order_items.quantity%type,
            price IN order_items.unit_price%type)
AS
BEGIN
    INSERT INTO order_items
    VALUES(orderId, itemId, productId, quant, price);
END;

*/

// C++ program
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <occi.h>
#include <cstring>
#include <iomanip>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;
double findProduct(Connection* conn, int product_id);
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);
int mainMenu();
int customerLogin(Connection* conn, int customerId);
int addToCart(Connection* conn, struct ShoppingCart cart[]);
void displayProducts(struct ShoppingCart cart[], int productCount);
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);
void run(Connection* conn);

struct ShoppingCart {
    int product_id;
    double price;
    int quantity;
};

int mainMenu() {
    int option;
    cout << "******************** Main Menu ********************" << endl;
    cout << "1)      Login" << endl;
    cout << "0)      Exit" << endl;
    cout << "Enter an option (0-1): ";
    cin >> option;
    while (option < 0 || option > 1) {
        cout << "******************** Main Menu ********************" << endl;
        cout << "1)      Login" << endl;
        cout << "0)      Exit" << endl;
        cout << "You entered a wrong value. Enter an option (0-1): ";
        cin >> option;
    }
    return option;
}

int customerLogin(Connection* conn, int customerId) {
    int found;
    try {
        Statement* stmt = conn->createStatement();
        stmt->setSQL("BEGIN find_customer(:1, :2); END;");
        stmt->setInt(1, customerId);
        int count;
        stmt->registerOutParam(2, Type::OCCIINT, sizeof(count));
        stmt->executeUpdate();
        found = stmt->getInt(2);
        if (found == 0)
            std::cout << "The customer does not exist.\n";
        conn->terminateStatement(stmt);
        return found;
    }
    catch(SQLException& sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }    
}

int addToCart(Connection* conn, struct ShoppingCart cart[]) {
    int count = 0;
    bool ok = false;
    int choose = 0;
    int result = 0;
    cout << "-------------- Add Products to Cart --------------" << endl;
    do {
        for (auto i = 0; i < 5; i++) {
            cout << "Enter the product ID: ";
            cin >> cart->product_id;
            cart->price = findProduct(conn, cart->product_id);
            if (cart->price != 0) {
                if (count == 5) {
                    cout << "Cart already had 5 items" << endl;
                    break;
                }
                cout << "Product Price: " << cart->price << endl;
                cout << "Enter the product Quantity: ";
                cin >> cart->quantity;
                ++count;
                cout << "Enter 1 to add more products or 0 to checkout: ";
                cin >> choose;
                if (choose == 0) {
                    break;
                }
            }
            else {
                cout << "The product does not exist. Try again..." << endl;
            }
        }
    } while (choose);
    return count;
}

double findProduct(Connection* conn, int product_id) {
    double price;    
    try {
        Statement* stmt = conn->createStatement();
        stmt->setSQL("BEGIN find_product(:1, :2); END;");
        stmt->setInt(1, product_id);
        int count;
        stmt->registerOutParam(2, Type::OCCIDOUBLE, sizeof(count));
        stmt->executeUpdate();
        price = stmt->getDouble(2);
        conn->terminateStatement(stmt);
    }
    catch (SQLException& sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }

    return price;
}

void displayProducts(struct ShoppingCart cart[], int productCount) {
    double total = 0;
    cout << "------- Ordered Products ---------" << endl;
    for (int i = 0; i < productCount; i++) {
        cout << "---Item " << i+1 << endl;
        cout << "Product ID: " << cart->product_id << endl;
        cout << "Price: " << cart->price << endl;
        cout << "Quantity: " << cart->quantity << endl;
        cout << "-----------------------" << endl;
        total += cart->price*cart->quantity;
    }
    //After display all products, calculate total
    cout << "Total: " << total << endl;
}

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
    char option;
    int check;
    do {
        cout << "Would you like to checkout? (Y/y or N/n) ";
        cin >> option;
        if (option != 'Y' && option != 'y' && option != 'N' && option != 'n') {
            cout << "Wrong input. Try again..." << endl;
        }
    } while (option != 'Y' && option != 'y' && option != 'N' && option != 'n');
    if (option == 'Y' || option == 'y') {
        try {
            Statement* stmt = conn->createStatement();
            stmt->setSQL("BEGIN add_order(:1, :2); END;");
            stmt->setInt(1, customerId);
            int new_order_id = { 0 };
            stmt->registerOutParam(2, Type::OCCIINT, sizeof(new_order_id));
            stmt->executeUpdate();
            new_order_id = stmt->getInt(2);

            for (int i = 0; i < productCount; i++)
            {
                stmt->setSQL("BEGIN add_order_item(:1, :2, :3, :4, :5); END;");
                stmt->setInt(1, new_order_id);
                stmt->setInt(2, i + 1);
                stmt->setInt(3, cart[i].product_id);
                stmt->setDouble(4, cart[i].quantity);
                stmt->setDouble(5, cart[i].price);
                stmt->executeUpdate();
            }
            conn->terminateStatement(stmt);
        }
        catch (SQLException& sqlExcp) {
            cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
        }
        check = 1;
        return check;
    }
    else {
        check = 0;
        return check;
    }
    
    
}

void run(Connection* conn) {
    int custID = 0;
    int out = 0;
    int option = 0;
    int found = 0;
    int prodCount = 0;
    int checkVal = 0;
    struct ShoppingCart cart[30];
    do {
        do {
            option = mainMenu();
            if (option == 1) {
                cout << "Enter the customer ID: ";
                cin >> custID;
                found = customerLogin(conn, custID);
                out = 1;
            }
            else {
                break;
            }
        } while (found != 0 && found != 1);
        if (option == 1 && found == 1) {
            prodCount = addToCart(conn, cart);
            displayProducts(cart, prodCount);
            checkVal = checkout(conn, cart, custID, prodCount);
            if (checkVal == 1) {
                cout << "The order is successfully completed." << endl;
            }
            else {
                cout << "The order is cancelled." << endl;
                option = 1;
            }
        }   
    } while (option);
    cout << "Good bye!..." << endl;
}

int main() {
    Environment* env = nullptr;
    Connection* conn = nullptr;
    string user = "dbs211_202a22";
    string pass = "66420391";
    string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";
    
    // try and catch
    try {
        env = Environment::createEnvironment(Environment::DEFAULT);
        conn = env->createConnection(user, pass, constr);
        run(conn);
       
        env->terminateConnection(conn);
        Environment::terminateEnvironment(env);
    }
    catch (SQLException& sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }

   
    return 0;
}