#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

struct connection_info {
	std::string name;
};

struct connection_handle {
	void send_data(const std::string& data) {
		std::cout << "Sent data: " << data << std::endl;
	}
};

struct connection_manager {
	connection_handle open(const connection_info& details) {
		std::cout << "Opened connection with name: " << details.name << std::endl;
		return connection_handle();
	}
};

class X {
  private:
	connection_info connection_details;
	connection_handle connection;

	std::once_flag connection_init_flag;

	void open_connection() {
		connection_manager manager;
		connection = manager.open(connection_details);
	}

  public:
	explicit X(const connection_info& details) : connection_details(details) {
		/*NOP*/
	}

	void send_data(const std::string& data) {
		std::call_once(connection_init_flag, &X::open_connection, this);
		connection.send_data(data);
	}
};

int main() {
	connection_info details{.name = "X_connection"};
	X x(details);

	std::vector<std::thread> pool;
	for (int i = 0; i < 10; ++i) {
		pool.push_back(std::thread([i, &x] () {
			x.send_data("Data from thread " + std::to_string(i));
		}));
	}

	std::for_each(pool.begin(), pool.end(), std::mem_fn(&std::thread::join));
}