
(define (onObjSelected obj)
  (if (equal? (gameobj-id obj) (gameobj-id mainobj))
    (begin
      (display "matching, id is: ")
      (display (gameobj-id mainobj))
      (display "\n")
    )
  )
)

(define (onMessage message)
  (display (string-append "message is: " message "\n"))
)