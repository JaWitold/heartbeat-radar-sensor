"""
Protocol encoders registry.
"""

from typing import Dict, Type
from ..interfaces import ISensorProtocol
from .ld2450 import LD2450Encoder
from .rd03d import RD03DEncoder
from .rcwl0516 import RCWL0516ArrayEncoder

PROTOCOLS: Dict[str, Type[ISensorProtocol]] = {
    "ld2450": LD2450Encoder,
    "rd03d": RD03DEncoder,
    "rcwl0516": RCWL0516ArrayEncoder,
}


def get_protocol(name: str) -> ISensorProtocol:
    name_lower = name.lower()
    if name_lower not in PROTOCOLS:
        raise ValueError(
            f"Unknown sensor protocol: '{name}'. Supported: {list(PROTOCOLS.keys())}"
        )
    return PROTOCOLS[name_lower]()
