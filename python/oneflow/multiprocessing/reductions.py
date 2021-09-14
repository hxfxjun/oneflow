from multiprocessing.reduction import ForkingPickler

import oneflow as flow
from oneflow.nn.parameter import Parameter
from oneflow.framework.tensor import Tensor


try:
    # Early load resource_sharer to prevent a partially initialized instance
    # from being inherited in a forked child process. The reduce_storage method
    # requires this module indirectly through DupFd(). The built-in mp.Queue
    # class pickles arguments in a background thread which may overlap with the
    # fork.
    import multiprocessing.resource_sharer
except ImportError:
    pass


def rebuild_tensor(cls, tensor_data, requires_grad):
    t = flow.tensor(tensor_data)
    if cls == Parameter:
        # we have to pass requires_grad into constructor, rather than set it as an
        # attribute later, because it's an important check for Integer Tensors to
        # have requires_grad=False (or else they raise an error)
        t = Parameter(t, requires_grad=requires_grad)
    else:
        t.requires_grad = requires_grad
    return t


def reduce_tensor(tensor):
    tensor_data = tensor.numpy()
    requires_grad = tensor.requires_grad
    return (rebuild_tensor, (type(tensor), tensor_data, requires_grad))

def reduce_local_tensor(tensor):
    tensor_data = tensor.numpy()
    requires_grad = tensor.requires_grad
    return (rebuild_tensor, (type(tensor), tensor_data, requires_grad))

def init_reductions():
    ForkingPickler.register(Tensor, reduce_tensor)
    ForkingPickler.register(flow._oneflow_internal.Tensor, reduce_local_tensor)
    ForkingPickler.register(Parameter, reduce_tensor)
    ForkingPickler.register(flow._oneflow_internal.nn.Parameter, reduce_local_tensor)