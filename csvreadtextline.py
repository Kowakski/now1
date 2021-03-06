import csv
import numpy as np
import tensorflow as tf

IRIS_TEST = "./woodata/iris_test.csv"
IRIS_TRAINING = './woodata/iris_training.csv'

COLUMNS = ['SepalLength','SepalWith','PetalLength','PetalWith','label']
FIELD_DEFAULTS = [[0.0],[0.0],[0.0],[0.0],[0]]

def __parse_line(line):   #"line" is the whole file
  fields = tf.decode_csv( line, FIELD_DEFAULTS )
  features = dict( zip( COLUMNS, fields ) )
  label = features.pop('label')
  return features, label

def data_input_function( PATH, BatchSize ):
    Data = tf.data.TextLineDataset( PATH, compression_type = None, buffer_size = None ).skip(1)
    Data = Data.map( __parse_line )
    Data = Data.shuffle( 1000 ).repeat( ).batch( BatchSize )
    return Data

#Define input function
# debuf:<class 'tensorflow.python.data.ops.readers.TextLineDataset'>

keys = ['SepalLength', 'SepalWith', 'PetalLength', 'PetalWith']

FeaturesColumns = [ tf.feature_column.numeric_column( key = s, shape=(1,), default_value=None, dtype = tf.float32, normalizer_fn = None ) for s in keys ]

#Define checkpoint config
MyCheckPointConfig = tf.estimator.RunConfig( save_checkpoints_secs = 1,   #Save checkpoint every 1 s
                                           keep_checkpoint_max = 100,     #Retain the 100 most recent checkpoints
                                            )

classifier = tf.estimator.DNNClassifier( hidden_units = [10, 20], feature_columns = FeaturesColumns, model_dir = "./storage", n_classes=3, weight_column = None, config = MyCheckPointConfig )

classifier.train( input_fn = lambda: data_input_function( IRIS_TRAINING, 20 ), hooks = None, steps = 200, max_steps = None, saving_listeners =  None )

EvaResult = classifier.evaluate( input_fn = lambda: data_input_function( IRIS_TEST, 20 ), steps=1, hooks=None, checkpoint_path=None, name=None )

def pred_input_fn( features, batchsize ):
    Dataset = tf.data.Dataset.from_tensor_slices(features)
    assert batchsize is not None, "Batch size is None"
    return Dataset.batch( batchsize )  #is predict set, do not shuffle it

#predict following set
predict_x = {
    'SepalLength': [5.1, 5.9, 6.9],
    'SepalWidth': [3.3, 3.0, 3.1],
    'PetalLength': [1.7, 4.2, 5.4],
    'PetalWidth': [0.5, 1.5, 2.1],
}
Prediction = classifier.predict( input_fn = lambda:pred_input_fn( predict_x, 200 ) )
print( "Prediction is {0}, {1}".format( type( Prediction ), Prediction ) )
# print(train_logits)修改为sess.run(tf.Print(train_logits,[train_logits]))后
