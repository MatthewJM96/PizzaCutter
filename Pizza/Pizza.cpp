#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <queue>

struct SliceDimensions {
	int width, height;
};

struct CentroidPosition {
	double x, y;
};
CentroidPosition operator+(const CentroidPosition& pos1, const CentroidPosition& pos2) {
	return { pos1.x + pos2.x, pos1.y + pos2.y };
}
CentroidPosition operator*(const CentroidPosition& pos, double i) {
	return { pos.x * i, pos.y * i };
}
CentroidPosition operator*(const CentroidPosition& pos, int i) {
	return pos * (double)i;
}
CentroidPosition operator/(const CentroidPosition& pos, double i) {
	return { pos.x / i, pos.y / i };
}
CentroidPosition operator/(const CentroidPosition& pos, int i) {
	return pos / (double)i;
}

struct Pizza {
	std::string data;
	SliceDimensions dims;
	int minToppings;
	int maxSliceSize;
};
using PizzaSlice = Pizza;

enum class Topping {
	MUSHROOM,
	TOMATO
};

enum class Direction {
	UP,
	RIGHT
};

/// Model that holds the charge data.
class PizzaLoader {
public:
	PizzaLoader() {}
	~PizzaLoader() {
		dispose();
	}

	void init(std::string filepath = "medium.in") {
		m_filepath = filepath;
	}
	void dispose() {
		// Close file is still open.
		if (m_file.is_open()) {
			m_file.close();
		}

		// Clear up pizza.
		m_pizza = {};
	}

	Pizza getPizza() {
		// If we haven't yet loaded data from the file, do so.
		if (empty(m_pizza.data)) {
			loadDataFromFile();
		}
		return m_pizza;
	}
private:
	bool openFile() {
		m_file.open(m_filepath, std::ios::in);
		return m_file.is_open();
	}
	
	void loadDataFromFile() {
		// If file isn't already open, and failed to open on an attempt, exit the program.
		if (!m_file.is_open() && !openFile()) {
			std::cout << "Could not open file: " << m_filepath << "." << std::endl
				<< "Exiting..." << std::endl;
			std::getchar();
			exit(0);
		}

		std::string metadata;
		std::getline(m_file, metadata);

		std::stringstream stream(metadata);

		stream >> m_pizza.dims.height;
		stream >> m_pizza.dims.width;
		stream >> m_pizza.minToppings;
		stream >> m_pizza.maxSliceSize;
		
		m_pizza.data = "";
		std::string line;
		while (std::getline(m_file, line)) {
			m_pizza.data.append(line);
		}
	}

	std::fstream m_file;
	std::string m_filepath;

	Pizza m_pizza;
};

class IMethod {
public:
	IMethod() {}
	~IMethod() {}

	virtual std::string solve(Pizza pizza) = 0;
};

class CutMethod : public IMethod {
public:
	std::string solve(Pizza pizza) {
		std::vector<PizzaSlice> slices;

		std::queue<PizzaSlice> pendingSlices;
		PizzaSlice currSlice = pizza;
		while (true) {
			PizzaSlice* s = tryCutsAndChoose(currSlice);
			if (s[0].data.size() <= pizza.maxSliceSize) {
				slices.push_back(s[0]);
			} else {
				pendingSlices.push(s[0]);
			}
			if (s[1].data.size() <= pizza.maxSliceSize) {
				slices.push_back(s[1]);
			} else {
				pendingSlices.push(s[1]);
			}
			if (pendingSlices.empty()) break;
			currSlice = pendingSlices.front();
			pendingSlices.pop();
		}

		std::cout << "AGUISDO";

		return "";
	}
private:
	PizzaSlice* tryCutsAndChoose(PizzaSlice slice, int offset = 0) {
		PizzaSlice* upSlices = makeCut(slice, Direction::UP, offset);
		PizzaSlice* rightSlices = makeCut(slice, Direction::RIGHT, offset);

		int mcUp1 = countTopping(upSlices[0], Topping::MUSHROOM);
		int mcUp2 = countTopping(upSlices[1], Topping::MUSHROOM);
		int tcUp1 = countTopping(upSlices[0], Topping::TOMATO);
		int tcUp2 = countTopping(upSlices[1], Topping::TOMATO);

		int mcRight1 = countTopping(rightSlices[0], Topping::MUSHROOM);
		int mcRight2 = countTopping(rightSlices[1], Topping::MUSHROOM);
		int tcRight1 = countTopping(rightSlices[0], Topping::TOMATO);
		int tcRight2 = countTopping(rightSlices[1], Topping::TOMATO);

		int upHowValid;
		bool up1Valid = mcUp1 >= slice.minToppings && tcUp1 >= slice.minToppings;
		bool up2Valid = mcUp2 >= slice.minToppings && tcUp2 >= slice.minToppings;
		upHowValid = (int)up1Valid + (int)up2Valid;

		int rightHowValid;
		bool right1Valid = mcRight1 >= slice.minToppings && tcRight1 >= slice.minToppings;
		bool right2Valid = mcRight2 >= slice.minToppings && tcRight2 >= slice.minToppings;
		rightHowValid = (int)right1Valid + (int)right2Valid;

		// Check if either produces invalid slices.
		if (upHowValid > rightHowValid) return upSlices;
		if (upHowValid < rightHowValid) return rightSlices;
		
		if (upHowValid < 2 && rightHowValid < 2) tryCutsAndChoose(slice, offset + 1);

		// Figure which choice of slices is better.
		int upQuality = std::pow(mcUp1 - mcUp2, 2) + std::pow(tcUp1 - tcUp1, 2);
		int rightQuality = std::pow(mcRight1 - mcRight2, 2) + std::pow(tcRight1 - tcRight1, 2);

		if (upQuality > rightQuality) {
			return upSlices;
		}
		return rightSlices;
	}

	PizzaSlice* makeCut(PizzaSlice slice, Direction dir, int offset = 0) {
		PizzaSlice* slices = new PizzaSlice[2];
		slices[0].data = "";
		slices[1].data = "";
		slices[0].maxSliceSize = slice.maxSliceSize;
		slices[1].maxSliceSize = slice.maxSliceSize;
		slices[0].minToppings = slice.minToppings;
		slices[1].minToppings = slice.minToppings;

		int mc = countTopping(slice, Topping::MUSHROOM);
		int tc = countTopping(slice, Topping::TOMATO);
		
		CentroidPosition mcentroid = getToppingCentroid(slice, Topping::MUSHROOM);
		CentroidPosition tcentroid = getToppingCentroid(slice, Topping::TOMATO);

		Topping t;
		if (mc > tc) {
			t = Topping::TOMATO;
		} else {
			t = Topping::MUSHROOM;
		}

		CentroidPosition wc = ((mcentroid * mc) + (tcentroid * tc)) / (double)(mc + tc);

		int cp;
		if (dir == Direction::UP) {
			cp = std::round(wc.x) + offset;
			if (cp < 1) cp = 1;

			slices[0].dims.height = slice.dims.height;
			slices[1].dims.height = slice.dims.height;

			slices[0].dims.width = cp - 1;
			slices[1].dims.width = slice.dims.width - cp + 1;

			int i;
			for (int y = 0; y < slice.dims.height; ++y) {
				for (int x = 0; x < slice.dims.width; ++x) {
					i = x + y * slice.dims.width;
					if (x < (cp - 1)) {
						slices[0].data.append(1, slice.data[i]);
					} else {
						slices[1].data.append(1, slice.data[i]);
					}
				}
			}
		} else {
			cp = std::round(wc.y) + offset;
			if (cp < 1) cp = 1;

			slices[0].dims.width = slice.dims.width;
			slices[1].dims.width = slice.dims.width;

			slices[0].dims.height = cp - 1;
			slices[1].dims.height = slice.dims.height - cp + 1;

			for (int y = 0; y < slice.dims.height; ++y) {
				if (y < (cp - 1)) {
					slices[0].data.append(slice.data.substr(y * slice.dims.width, slice.dims.width));
				} else {
					slices[1].data.append(slice.data.substr(y * slice.dims.width, slice.dims.width));
				}
			}
		}
		return slices;
	}

	int countTopping(PizzaSlice slice, Topping topping) {
		char t;
		switch (topping) {
		case Topping::MUSHROOM:
			t = 'M';
			break;
		case Topping::TOMATO:
			t = 'T';
			break;
		}

		int c = 0;

		int i;
		for (int x = 0; x < slice.dims.width; ++x) {
			for (int y = 0; y < slice.dims.height; ++y) {
				i = x + y * slice.dims.width;
				if (slice.data[i] == t) {
					++c;
				}
			}
		}

		return c;
	}

	CentroidPosition getToppingCentroid(PizzaSlice slice, Topping topping) {
		char t;
		switch (topping) {
		case Topping::MUSHROOM:
			t = 'M';
			break;
		case Topping::TOMATO:
			t = 'T';
			break;
		}

		int c = 0;
		double xAvg = 0.0, yAvg = 0.0;

		int i;
		for (int x = 0; x < slice.dims.width; ++x) {
			for (int y = 0; y < slice.dims.height; ++y) {
				i = x + y * slice.dims.width;
				if (slice.data[i] == t) {
					xAvg = ((xAvg * c) + ((double)x + 0.5)) / (c + 1);
					yAvg = ((yAvg * c) + ((double)y + 0.5)) / (c + 1);
					++c;
				}
			}
		}

		return {xAvg, yAvg };
	}
};

class PointExpandMethod : public IMethod {
public:
	std::string solve(Pizza pizza) {
		return "";
	}
};

class PizzaSliceSolver {
public:
	PizzaSliceSolver() {}
	~PizzaSliceSolver() {}

	enum class Method {
		CUT,
		POINT_EXPAND
	};

	void init(Pizza pizza) {
		m_pizza = pizza;
	}

	void solve(Method method) {
		m_result = "";

		IMethod* m = nullptr;
		switch (method) {
		case Method::CUT:
			m = new CutMethod();
			break;
		case Method::POINT_EXPAND:
			m = new PointExpandMethod();
			break;
		}
		m_result = m->solve(m_pizza);
	}

	std::string getResult() {
		return m_result;
	}
private:
	std::string m_result;
	Pizza m_pizza;
};

int main() {
	PizzaLoader loader;
	loader.init();
	
	Pizza pizza = loader.getPizza();

	PizzaSliceSolver solver;
	solver.init(pizza);

	solver.solve(PizzaSliceSolver::Method::CUT);
	
	std::cout << "BASHJDAPD";
}