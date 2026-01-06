# 4.Jan.2026
#import time
#import warnings
#warnings.filterwarnings('ignore')
#import matplotlib.pyplot as plt
from micromlgen import port
import numpy as np
import pandas as pd
from sklearn import tree
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier

df = pd.read_csv("Traindata_WZ_28-11-2025_v2.csv", sep=";", decimal=',', header=None) # Trainingsdaten als CSV laden
df = df.sample(frac=1.0).reset_index(drop=True) # Mischen

x_train = df.iloc[:, 0:4]
y_train = df.iloc[:, -1]

print("Train Data (X):")
print(x_train.head())

print("\nTrain Targets (y):")
print(y_train.head())

train_features, test_features, train_types, test_types = train_test_split(x_train, y_train, train_size=0.8, random_state=1)

print("Train data: ")
print(train_features.shape)
print("Test data: ")
print(test_features.shape)

forest = RandomForestClassifier(criterion='gini', n_estimators=100, max_depth=6, random_state=1, n_jobs=4) # 100 Bäume
forest.fit(train_features, train_types)

f1 = open("RF_100.p", "w")
for i, decisiontree in enumerate(forest.estimators_):
    node_counts = decisiontree.tree_.value
    p = node_counts / node_counts.sum(axis=2, keepdims=True)    
    leaves = np.where(decisiontree.tree_.children_left == -1)[0]
    f1.write(str(leaves) + "\n")
    # print("i: " + str(i))
    for leaf in leaves:
        probs = p[leaf][0]
        # print(f"Leaf {leaf}: p = {probs}") 
        f1.write(str(probs) + "\n")
f1.close()

f2 = open("RF_100.h", "w")
f2.write(port(forest))
f2.close()

feature_dim=4
max_samples=1000
X_lim = test_features.iloc[:max_samples]
y_lim = test_types.iloc[:max_samples]
dataset_size = len(X_lim)

f3 = open("RF_100_testset.h", "w")
f3.write("#pragma once\n")
f3.write(f"#define FEATURES_DIM {feature_dim}\n")
f3.write(f"#define DATASET_SIZE {dataset_size}\n\n")
f3.write("static float X[DATASET_SIZE][FEATURES_DIM] = {\n")
for row in X_lim.values:
    values = ", ".join(f"{v:.3f}" for v in row)
    f3.write(f"    {{ {values} }},\n")
f3.write("};\n\n")
f3.write("static int y[DATASET_SIZE] = {\n    ")
f3.write(", ".join(str(int(label)) for label in y_lim.values))
f3.write("\n};\n")
f3.close()

import emlearn
cmodel = emlearn.convert(forest, method=None, dtype='float')
code_emlearn = cmodel.save(file="RF_100_emlearn.h", name='model')

import m2cgen
code_m2cgen = m2cgen.export_to_c(forest, indent=4, function_name="model_m2cgen")
f4 = open("RF_100_m2cgen.h", "w")
f4.write(code_m2cgen)
f4.close()

#from sklearn_porter import port
#code_sp = port(forest, language='js', template='attached')
#f5 = open("RF_100_sklearn_porter.js", "w")
#f5.write(code_sp)
#f5.close()

#fig, axes = plt.subplots(nrows = 1,ncols = 1,figsize = (8,2.5), dpi=100)
#tree.plot_tree(forest.estimators_[10],
#               feature_names=["Bewegung", "Lautstärke", "Licht", "Temperatur"],
#               class_names=["Inaktiv (Schlaf)", "Mann anwesend", "Frau anwesend"],               
#               filled = True,
#               rounded=True);
#fig.savefig('rf_individualtree.png')
#plt.show()
