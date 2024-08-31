import os
import re
from collections import defaultdict

# Regular expression to match #include directives
include_pattern = re.compile(r'#include\s*["<](.*)[">]')

def find_includes(file_path, search_folder):
    """Find all includes in a given file that exist within the search folder."""
    includes = []
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                match = include_pattern.match(line.strip())
                if match:
                    include_file = match.group(1)
                    include_path = find_include_path(include_file, search_folder)
                    if include_path:
                        includes.append(include_path)
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
    return includes

def find_include_path(include_file, search_folder):
    """Find the full path of an included file within the specified folder."""
    for subdir, _, files in os.walk(search_folder):
        if include_file in files:
            return os.path.join(subdir, include_file)
    return None

def build_dependency_graph(file_path, search_folder, graph, visited):
    """Recursively build a dependency graph starting from the specified file."""
    if file_path in visited:
        return  # Prevent re-processing the same file
    visited.add(file_path)

    includes = find_includes(file_path, search_folder)
    current_file = os.path.relpath(file_path, search_folder)

    for include_path in includes:
        include_file = os.path.relpath(include_path, search_folder)
        graph[current_file].append(include_file)
        build_dependency_graph(include_path, search_folder, graph, visited)

def detect_cycles_util(node, visited, stack, graph):
    """Utility function to detect cycles using DFS."""
    visited[node] = True
    stack[node] = True

    for neighbor in graph[node]:
        if not visited.get(neighbor, False):
            if detect_cycles_util(neighbor, visited, stack, graph):
                return True
        elif stack.get(neighbor, False):
            print(f"Cycle detected: {neighbor} -> {node}")
            return True

    stack[node] = False
    return False

def detect_cycles(graph):
    """Detect cycles in the dependency graph."""
    visited = {}
    stack = {}

    # Iterate over a static list of keys to avoid modifying the graph during iteration
    for node in list(graph.keys()):
        if not visited.get(node, False):
            if detect_cycles_util(node, visited, stack, graph):
                return True
    return False

def main():
    main_file = input("Enter the path to your main.cpp file: ")
    search_folder = input("Enter the path to the folder where your project files are located: ")

    # Initialize the dependency graph and visited set
    graph = defaultdict(list)
    visited = set()

    # Build the dependency graph starting from the main.cpp file
    build_dependency_graph(main_file, search_folder, graph, visited)
    
    # Detect cycles in the graph
    if detect_cycles(graph):
        print("Circular dependencies detected!")
    else:
        print("No circular dependencies found.")

if __name__ == "__main__":
    main()
