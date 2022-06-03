from grid import Grid

# Python3 program to illustrate
# Expectimax Algorithm

# Version Beta is mostly written for the sake of understanding expectimax implementation
  
# Structure to declare
# left and right nodes
class Bot:
    def __init__(self, grid):
        self.grid = grid

    def get_move(self):
        #print("Grid sent from 2048: ", self.grid, " with shape: ")
        #self.grid.print_grid()
        node = Node(self.grid)
        utility_options = []
        #print("Original grid has id: ", self.grid, " with shape: ")
        #self.grid.print_grid()
        for option in node.options:
            #print("Option: ", option, " has grid")
            #option[0].print_grid()
            utility_options.append((expectimax(node, False, 0), option[1]))
        max_utility = max(utility_options, key=lambda x:x[0])
        return max_utility[1]
    
class Node:
    def __init__(self, grid):
        self.grid = grid
        self.value = grid.current_score
        # Make copies
        self.options = []
        #Up copy
        up_grid = self.grid.copy()
        if (up_grid):   
            up_grid.clear_flags()
            up_grid.grid_up()
            if (up_grid.cells != self.grid.cells):
                self.options.append((up_grid, "Up"))
        #Left copy
        left_grid = self.grid.copy()
        if (left_grid):   
            left_grid.clear_flags()
            left_grid.grid_left()
            if (left_grid.cells != self.grid.cells):
                self.options.append((left_grid, "Left"))
        #Down copy
        down_grid = self.grid.copy()
        if (down_grid):   
            down_grid.clear_flags()
            down_grid.grid_down()
            if (down_grid.cells != self.grid.cells):
                self.options.append((down_grid, "Down"))
        #Right copy
        right_grid = self.grid.copy()
        if (right_grid): 
            right_grid.clear_flags()  
            right_grid.grid_right()
            if (right_grid.cells != self.grid.cells):
                self.options.append((right_grid, "Right"))
        # I have no idea why it reverses itself after this, but the following line fixes it
        self.grid.reverse()
        
        
MAX_DEPTH = 4

# Getting expectimax
def expectimax(node, is_max, depth):
 
    # Condition for Terminal node
    if (depth == MAX_DEPTH or not node.options):
        return node.value
     
    # Maximizer node. Chooses the max from the 4 move subtrees
    if (is_max):
        options = []
        for option in node.options:
            options.append(expectimax(Node(option[0]), False, depth+1))
        return max(options)
  
    # Chance node. Returns the average of possible panel spawns
    else:
        spots = node.grid.retrieve_empty_cells()
        expectation_sum = 0
        if (spots):
            for spot in spots:
                # Create the grid representing the possibility a 4 might show up in this empty cell
                four_possibility = node.grid.copy()
                four_possibility.cells[spot[0]][spot[1]] = 4
                # Create the grid representing the possibility a 2 might show up in this empty cell
                two_possibility = node.grid.copy()
                # I dont really know why this happens yet, but sometimes two_possitibility is bool and crashes
                if (type(two_possibility) == Grid):
                    two_possibility.cells[spot[0]][spot[1]] = 2
                    expectation_sum += (0.9*expectimax(Node(two_possibility), True, depth+1) + 
                                        0.1*expectimax(Node(four_possibility), True, depth+1))
            return expectation_sum/len(spots)
        else:
            return 0
     
