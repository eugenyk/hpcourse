namespace Carramba.Yarr
{
    partial class FormMainYarr
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMainYarr));
            this.buttonSend = new System.Windows.Forms.Button();
            this.textBoxICanTalk = new System.Windows.Forms.TextBox();
            this.textBoxICanSee = new System.Windows.Forms.TextBox();
            this.username = new System.Windows.Forms.Label();
            this.usernametext = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // buttonSend
            // 
            this.buttonSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSend.Location = new System.Drawing.Point(283, 214);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 0;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // textBoxICanTalk
            // 
            this.textBoxICanTalk.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxICanTalk.Location = new System.Drawing.Point(13, 215);
            this.textBoxICanTalk.Name = "textBoxICanTalk";
            this.textBoxICanTalk.Size = new System.Drawing.Size(264, 20);
            this.textBoxICanTalk.TabIndex = 1;
            this.textBoxICanTalk.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.textBoxICanTalk_PreviewKeyDown);
            // 
            // textBoxICanSee
            // 
            this.textBoxICanSee.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxICanSee.Location = new System.Drawing.Point(13, 12);
            this.textBoxICanSee.Multiline = true;
            this.textBoxICanSee.Name = "textBoxICanSee";
            this.textBoxICanSee.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxICanSee.Size = new System.Drawing.Size(345, 197);
            this.textBoxICanSee.TabIndex = 2;
            // 
            // username
            // 
            this.username.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.username.AutoSize = true;
            this.username.Location = new System.Drawing.Point(12, 249);
            this.username.Name = "username";
            this.username.Size = new System.Drawing.Size(56, 13);
            this.username.TabIndex = 5;
            this.username.Text = "Погоняло";
            // 
            // usernametext
            // 
            this.usernametext.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.usernametext.Location = new System.Drawing.Point(68, 246);
            this.usernametext.Name = "usernametext";
            this.usernametext.Size = new System.Drawing.Size(100, 20);
            this.usernametext.TabIndex = 6;
            this.usernametext.Text = "J.Silver";
            // 
            // FormMainYarr
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(373, 282);
            this.Controls.Add(this.usernametext);
            this.Controls.Add(this.username);
            this.Controls.Add(this.textBoxICanSee);
            this.Controls.Add(this.textBoxICanTalk);
            this.Controls.Add(this.buttonSend);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "FormMainYarr";
            this.Text = "Yarr Messenger";
            this.Shown += new System.EventHandler(this.FormMainYarr_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.TextBox textBoxICanTalk;
        private System.Windows.Forms.TextBox textBoxICanSee;
        private System.Windows.Forms.Label username;
        private System.Windows.Forms.TextBox usernametext;
    }
}

