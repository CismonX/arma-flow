# arma-flow

A simple power flow calculator using Newton's method, written in C++ using armadillo.

Additional support for three-phase short-circuit calculation.

## 1. Requirements

* A newer version of [Armadillo](http://arma.sourceforge.net/).
  * Tested on 8.400.0
* The [Janus](https://github.com/dmulholland/janus-cpp) library.
* Compiler with C++17 support.

### 1.1 Build Janus

You need to build and install Janus before building arma-flow.

For example:

```bash
git clone https://github.com/dmulholland/janus-cpp.git
cd janus-cpp/src
g++ -std=c++11 -shared -fPIC -o libjanus.so janus.cpp
sudo cp libjanus.so /usr/lib
sudo cp janus.h /usr/include
```


## 2. Documentation

### 2.1 CLI options

* `-h | --help` : Prints the usage of arma-flow and exit.
* `--version` : Prints the version of arma-flow and exit.
* `-o <output_file_prefix>` : Prefix to output file. (Can be relative path)
* `-n <node_data_file>` : Path to node data file. (Can be relative path)
* `-e <edge_data_file>` : Path to edge data file. (Can be relative path)
* `-r` : Remove first line of input CSV files before parsing.
* `-i <max_iterations>` : Max number of iterations to be performed before aborting.
* `-a <accuracy>` : Max deviation to be tolerated.
* `-s <node_id>` : Calculate three-phase short circuit on specified node.
* `--ignore-load` : Ignore load current when calculating three-phase short circuit.
* `--tr <transition_impedance(real)>` : Transition impedance of three-phase short circuit(real part).
* `--ti <transition_impedance(imag)>` : Transition impedance of three-phase short circuit(imaginary part).
* `-v | --verbose` : Output more text to STDOUT.

For example:

```bash
./arma-flow -n examples/3-gen-9-nodes/nodes.csv \
            -e examples/3-gen-9-nodes/edges.csv \
            -r -s 4 --ignore-load --verbose
```

### 2.2 Input

Input node data file and edge data file should be of CSV format. Definition of each column is shown below:

#### 2.2.1 Node data file

* Node voltage (PV nodes and swing node)
* Generator power (active power, PV nodes)
* Load power (active power, PQ and PV nodes)
* Load power (reactive power, PQ nodes)
* (When three-phase short circuit calculation is enabled) Generator admittance
* Node type (0 - swing node, 1- PQ node, 2 - PV node)

#### 2.2.2 Edge data file

* First node id (equal to node data row offset, start at 1)
* Second node id (equal to node data row offset, start at 1)
* Resistance (real) -- R
* Resistance (imaginary) -- X
* Gounding admittance (divided by two) -- B/2
* Transformer ratio -- k

### 2.3 Output

The node admittance matrix and result of power flow calculation of the given system will be written to CSV files. If in verbose mode, some temporary data during calculation is printed to STDOUT.

* Note that the nodes are sorted before calculation, sequence of nodes in some verbose output may not be the same as input file.

#### 2.3.1 Node admittance matrix

The real part and imaginary part of node admittance matrix will be printed to "\<prefix\>node-admittance-real.csv" and "\<prefix\>node-admittance-imag.csv".

#### 2.3.2 Calculation result

The result of power flow calculation will be printed to "\<prefix\>flow.csv". The definition of each column is given below:

* Node voltage
* Phase angle of node voltage (Radian)
* Node power (active)
* Node power (reactive)

#### 2.3.3 Short circuit calculation

The node impedance matrix will be printed to "\<prefix\>node-impedance-real.csv" and "\<prefix\>node-impedance-imag.csv".

Short circuit current will be printed directly to STDOUT.

Node voltage after short circuit will be printed to "\<prefix\>short-circuit-voltage.csv", and edge current "\<prefix\>short-circuit-edge-current.csv".