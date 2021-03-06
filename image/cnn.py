'''
@Create a customer estimator for cnn network
'''
import numpy as np
import tensorflow as tf
import tensorflow.contrib.eager as tfe
# import logging

# tfe.enable_eager_execution()
# from tensorflow.python import debug as tf_debug
tf.logging.set_verbosity(tf.logging.INFO)
np.set_printoptions(threshold=np.nan)
# tf.logging._logger.basicConfig(filename='tensorflow.log', level=logging.DEBUG)
# print(ndarray)

#define estimator function
def mod_fn( features, labels, mode ):
    print("Here feature is:{0}".format( features ) )
    inputs = tf.reshape(features['x'], [-1, 28, 28, 1])
    # inputs = features
    print("inputs is:{0}".format(inputs))
    # tf.logging.log( tf.logging.INFO,  inputs )

    #convolution layer
    conv1 = tf.layers.conv2d( inputs = inputs,
                              filters = 32,
                              kernel_size = [5,5],
                              padding = 'same',
                              activation=tf.nn.relu, name = 'conv1')
    print("conv1 is:{0}".format(conv1))
    # tf.logging.log( tf.logging.INFO, conv1 )
    #pooling layer
    pool1 = tf.layers.average_pooling2d( inputs = conv1, pool_size = ( 2, 2 ), strides = [2,2], data_format='channels_last', name = 'pool1' )
    print("pool1 is:{0}".format(pool1))
    # tf.logging.log( tf.logging.INFO, pool1 )


    #convolution layer
    conv2 = tf.layers.conv2d(  inputs = pool1,
                               filters = 64,
                               kernel_size = [5,5],
                               padding = 'same',
                               activation=tf.nn.relu )
    print("conv2 is:{0}".format(conv2))
    #pooling layer
    pool2 = tf.layers.average_pooling2d( inputs = conv2, pool_size = ( 2, 2 ), strides = 2 )

    print("pool2 is:{0}".format(pool2))
    #full connected layer
    FullConnet = tf.reshape( pool2, [-1, 7*7*64] )  #7width*7height*64channel

    dense = tf.layers.dense( inputs = FullConnet, units = 7*7*64, activation = tf.nn.relu )

    #softmax layer
    logits = tf.layers.dense( inputs = dense, units = 10 )
    print("sln logits is {0}".format(logits))

    #predict value
    predict = tf.argmax(logits, 1)
    print("sln predict is {0}".format( predict ))

    #losses
    loss = tf.losses.sparse_softmax_cross_entropy(labels =  labels, logits = logits)

    if mode == tf.estimator.ModeKeys.PREDICT:
        return tf.estimator.EstimatorSpec( mode, predictions = predict )

    if mode == tf.estimator.ModeKeys.TRAIN:
        optimizer = tf.train.AdagradOptimizer( learning_rate = 0.001 )
        train_op = optimizer.minimize(
            loss = loss,
            global_step = tf.train.get_global_step() )
        return tf.estimator.EstimatorSpec( mode, loss = loss, train_op = train_op )

    if mode == tf.estimator.ModeKeys.EVAL:
        accuracy = tf.metrics.accuracy( labels = labels, predictions = predict )
        metrics = {'accuracy':accuracy}
        return tf.estimator.EstimatorSpec( mode, loss = loss, eval_metric_ops = metrics )

def data_input_fn( data, labels ):
    result = tf.estimator.inputs.numpy_input_fn( x = data, y = labels, batch_size = 100, shuffle = True )
    tf.logging.log(tf.logging.INFO, result)
    return result

# "con1": "conv1/Relu"
tensors_to_log = { "ada":"Adagrad/value"}     #Get the tensor name form tensorboard graph
logging_hook = tf.train.LoggingTensorHook(
  tensors=tensors_to_log, every_n_iter=50)


#define main function
def main(unused_args):
    data = tf.contrib.learn.datasets.mnist.load_mnist()
    TrainImages = {'x':data.train.images}
    # TrainLabels = data.train.labels
    TrainLabels =  np.asarray(data.train.labels, dtype=np.int32)

    EvaluImages = {'x':data.validation.images}
    EvaluLabels = np.asarray(data.validation.labels, dtype=np.int32)
    print("evaluimage is {0}".format(data.validation.images.shape) )
    print("EvaluLabels is {0}".format(EvaluLabels.shape) )

#Create estimator
    estimator = tf.estimator.Estimator(
        model_fn = mod_fn,
        model_dir = './cnn',
        )

#Train
    train_data_input_fn = tf.estimator.inputs.numpy_input_fn(
        x = TrainImages,
        y = TrainLabels,
        batch_size = 100,
        num_epochs=None,
        shuffle = True )

    # train = estimator.train( input_fn = lambda:data_input_fn( data = TrainImages, labels = TrainLabels ), steps = 20 )
    train = estimator.train(
     input_fn = train_data_input_fn,
     # hooks = [logging_hook],
     hooks = None,
     steps = 2000 )

#Evaluation
    eva_data_input_fn =  tf.estimator.inputs.numpy_input_fn(
        x = EvaluImages,
        y = EvaluLabels,
        shuffle = True )
    evalutaion = estimator.evaluate( input_fn = eva_data_input_fn, steps = 1 )
    print("Evaluation is:{0}".format(evalutaion))

if __name__ == "__main__":
    tf.app.run()