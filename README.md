Digital Fabric (DF) is a neural computing system employing multiple processing
elements that communicate using a packet switched network. The system level
architecture is inspired by the cortical areas found in the brain. These areas are
roughly arranged by function and tend to be connected in a hierarchical manner with
many feedback loops. Furthermore, cortex is arranged as a highly folded 2D sheet
with mainly cell bodies making up gray matter and axons making up the connections
between cells or white matter. The cell bodies in cortex are further segmented into
columns which are thought to provide a modular computing element.

Like cortex, the DF is arranged as a 2D network of interconnected modular comput-
ing elements. The Digital Fabric Node (DFN) consists of an individual processing
element or Macro-Circuit Module (MM) connected to a dedicated Routing Module
(RM). Digital fabric nodes are organized into a planar torus with ordinal nearest
neighbor connections.

The MM internal structure consists of a spiking neural network model. This
model generates spike events which have a specific source and destination. These
events are encoded by the RM into packets. This process is called Address Event
Representation (AER).
