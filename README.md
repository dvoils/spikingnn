# Spiking Neural Network

This is a spiking neural network which simulates a neural computing system employing multiple processing elements. These elements or "nodes" communicate using a packet switched network called a "digital fabric" or DF. The system level architecture is inspired by the cortical areas found in the brain. These areas are roughly arranged by function and tend to be connected in a hierarchical manner with many feedback loops. Furthermore, cortex is arranged as a highly folded 2D sheet with mainly cell bodies making up gray matter and axons making up the connections between cells or white matter. The cell bodies in cortex are further segmented into columns which are thought to provide a modular computing element.

Like cortex, the DF is arranged as a 2D network of interconnected modular computing elements. The Digital Fabric Node (DFN) consists of an individual processing
element or Macro-Circuit Module (MM) connected to a dedicated Routing Module (RM). Digital fabric nodes are organized into a planar torus with ordinal nearest
neighbor connections.

The MM internal structure consists of a spiking neural network model. This model generates spike events which have a specific source and destination. These
events are encoded by the RM into packets. This process is called Address Event Representation (AER). This project is actually a clock-accurate specification that was later used to create an FPGA implementation written in Verilog.

Many thanks to the DARPA SyNAPSE team for funding this project.

# Getting Started

The project has 2 parts:
1. A C++ back-end that implements the routing fabric.
2. A Python front end that is used to control the simulation and graphically display the result.

To compile the C++ code, just type "make". The python front-end and C++ back end were designed to communicate over a private network using TCP/IP protocol.
