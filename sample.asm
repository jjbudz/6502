$40	.DATA	$06 $55 $AA $AB

$4000	  LDXZ  $40
LOOP	  DEX
	      BNE	  LOOP
        BRK
