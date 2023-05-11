

(define disableInput #f)
(define (onKey key scancode action mods)
	(format #t "key is: ~a\n" key)
  (if (and (= key 48) (= action 1))
  	(begin
  		(set! disableInput (not disableInput))
   		(set-wstate (list
   			(list "editor" "disableinput" (if disableInput "true" "false"))
   		))
  	)
  )
)