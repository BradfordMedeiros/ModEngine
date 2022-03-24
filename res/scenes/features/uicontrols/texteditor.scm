
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
        (if (= key 257)
          (set! currentText (string-append currentText "\n"))
          (begin
            (format #t "key is ~a ~a\n" key (string (integer->char key)))
            (set! currentText (string-append currentText (string (integer->char key))))
          )
        )

      )
      (if (= key 345)
        (write-tofile)
      )
      (updateText currentText)
    )
  )
)


(define (write-tofile)
  (let ((output-port (open-file "modtext.txt" "w")))
    (display currentText output-port)
    (newline output-port)
    (close output-port)
  )
)
