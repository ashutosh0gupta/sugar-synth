import numpy as np
import argparse
import binarytree as bt
def flattened(root):
    if root.left!=None and root.right!=None:
        return "(a " + flattened(root.left) + " " + flattened(root.right) + ")"
    elif root.left==None and root.right==None:
        return "_"
    elif root.left==None:
        return "(b " +flattened(root.right) + ")"
    elif root.right==None:
        return "(b " +flattened(root.left) + ")"
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed')
    parser.add_argument('--exno')
    args = parser.parse_args()
    seed = args.seed
    exno = args.exno
    np.random.seed(int(seed))
    with open("examples/ex{}.sugar".format(int(exno)),"w") as f:
        f.write("(number-of-rules-to-learn {})\n".format(5))
        f.write("(max-rule-depth {})\n".format(4))
        f.write("(max-compartments {})\n".format(1))
        f.write("(sugar a 2)\n")
        f.write("(sugar b 1)\n")
        f.write("(seed-mol a)\n")
        mytree = bt.tree(6)
        res = [i for i in mytree.inorder if i not in mytree.leaves]
        idx = np.random.choice(len(res),5)
        subtrees = []
        for i in idx:
            subtrees.append(res[i])
        for i in subtrees:
            f.write("(mol " + flattened(i) + " )\n")
