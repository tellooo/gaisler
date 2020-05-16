echo Setting up the environment for debugging l8.\n

set history filename .gdbhistory
set history save on

#python execfile('l8.py') 
break perl2.c:211
#break l8_perl_nexttoken
#break perl_indirobj
#break 0_ptr.c:186
#break test.c:18

define pstr
  set $i = 0
  while ($i != 4) 
    printf "%d:%c\n", $i, $arg0->ptr[$i]
    set $i++
  end
end

define v2
  var $a=1
  printf "v2.a:%d\n", $a
end

define do_car
  var $v=((struct _l8cell *)$arg0)->u_p.n.p[0].id
  define_return $v
end

define car
  var $v=((struct _l8cell *)$arg0)->u_p.n.p[0].id
  define_return $v
end

define v1
  var $a=2
  v2
  printf "v1.a:%d\n", $a
end


define l1
  define_return 1
end

define l2
  var $a
  p l1()
end

define pid
   set dbgid(cctx, $arg0, 0)
end

define pstack
   set dbg_stk_print(cctx,0)
   printf "\n"
end

define pcell
  set dbg_out_car(stderr,$arg0)
  printf "\n"
end

define ppa
  var $i = 8
  set $a = substr($arg0->buf->str.ptr,0,$arg0->buf->str.cur)+"#¶"+substr($arg0->buf->str.ptr,$arg0->buf->str.cur) 
  p $a 
  printf "["
  while ($i != 0) 
    if ($arg0->tokcnt-$i >= 0) 
      if ($arg0->tokcnt-$i == $arg0->tokpos)
	printf "<"
      end
      printf "%s", tostr($arg0->tokens[$arg0->tokcnt-$i].tok)
      if ($arg0->tokcnt-$i == $arg0->tokpos)
	printf ">"
      end
      printf " "
    end
    set $i--
  end
  printf "]\n"

end
