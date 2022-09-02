(define (onKeyChar codepoint) 
  (define objId (gameobj-id mainobj))
  (format #t "object id: ~a\n" objId)
  ;(define exposureAmount (next-exposure))
  (if (equal? codepoint 112)  ; p
    (set-wstate (list 
      (list "editor" "selected-index" (number->string objId))  ; to float loses precision?
    ))
  )
)
