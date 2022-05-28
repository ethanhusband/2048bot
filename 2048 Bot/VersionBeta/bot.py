# Python3 program to illustrate
# Expectimax Algorithm
  
# Structure to declare
# left and right nodes
class Bot:
    def __init__(self, grid):
        self.grid = grid

    def get_move():
        return
    
class Node:
    def __init__(self, grid):
        self.grid = grid
        self.score = grid.current_score
        

# Getting expectimax
def expectimax(node, is_max, depth):
 
    # Condition for Terminal node
    if (depth == 8 or (node.left == None and node.right == None)):
        return node.value
     
    # Maximizer node. Chooses the max from the
    # left and right sub-trees
    if (is_max):
        return max(expectimax(node.left, False), expectimax(node.right, False))
  
    # Chance node. Returns the average of
    # the left and right sub-trees
    else:
        return (expectimax(node.left, True)+ expectimax(node.right, True))/2
     