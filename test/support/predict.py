import numpy as np
from isotree import IsolationForest

X = np.loadtxt('test/support/data.csv', delimiter=',')
model = IsolationForest(ntrees=10, ndim=2, nthreads=1)
model.fit(X)
predictions = model.predict(X)
print(predictions[0])
print("Point with highest outlier score: ", X[np.argsort(-predictions)[0], ])
