(define (onKeyChar codepoint) 
  (define objId (gameobj-id mainobj))
  (format #t "object id: ~a\n" objId)
  (if (equal? codepoint 112)  ; p
    (set-wstate (list 
      (list "editor" "selected-index" (number->string objId))  ; to float loses precision?
    ))
  )
)
