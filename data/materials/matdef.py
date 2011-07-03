# Copyright 2011 Branan Purvine-Riley and Adam Johnson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import SensEngine
from SensEngine import MaterialDef

def register_material(name, mat):
    try:
        SensEngine.client.loader.add_material(mat, name)
        return None
    except AttributeError: # we want to forward any errors that might have come from the pipeline
        pass
    raise NotImplementedError("Don't understand this context; can't register materials")
