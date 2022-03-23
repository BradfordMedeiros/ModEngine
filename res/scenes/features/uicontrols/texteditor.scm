
(enforce-layout (gameobj-id mainobj))


(define (updateText value)
  (gameobj-setattr! (lsobj-name ")sometext") 
    (list (list "value" value))
  )
)

(define currentText "")
(define (lessIndex) (max 0 (- (string-length currentText) 1)))
(define (onKey key scancode action mods)
  (format #t "key is ~a\n" key)
  (if (= action 0)
    (begin
      (if (= key 259)
        (set! currentText (substring currentText 0 (lessIndex)))
        (begin
          (format #t "key is ~a ~a\n" key (string (integer->char key)))
          (set! currentText (string-append currentText (string (integer->char key))))
        )
      )
      (updateText currentText)
    )
  )
)